/*
 * pb_io.c: handle incoming plipbox packets
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plipbox.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "global.h"
   
#include "net/net.h"
#include "net/eth.h"
#include "net/ip.h"
#include "net/arp.h"

#include "pkt_buf.h"
#include "enc28j60.h"
#include "pb_proto.h"
#include "uart.h"
#include "uartutil.h"
#include "param.h"
#include "dump.h"
#include "timer.h"
#include "stats.h"
#include "log.h"

static u16 offset;

// SANA driver parameters sent in magic packet
u08 sana_online;
u08 sana_mac[6];
u08 sana_version[2];

// ----- send callbacks -----

static void rx_begin(u16 *pkt_size)
{
  // start writing packet with eth header
  offset = 0;
  enc28j60_packet_tx_begin_range(0);
}

static void rx_data(u08 *data)
{
  // clone packet to our packet buffer
  if(offset < PKT_BUF_SIZE) {
    rx_pkt_buf[offset] = *data;
    offset ++;
  }
  
  // always copy to TX buffer of enc28j60
  enc28j60_packet_tx_byte(*data);
}

static void rx_end(u16 pkt_size)
{
  // end range in enc28j60 buffer
  enc28j60_packet_tx_end_range();
  rx_pkt_size = pkt_size;
}

// ----- recv callbacks -----

static void tx_begin(u16 *pkt_size)
{
  // report size of packet
  *pkt_size = tx_pkt_size;
  offset = 0;
  enc28j60_packet_rx_byte_begin();
}

static void tx_data(u08 *data)
{
  // clone packet to our packet buffer
  if(offset < PKT_BUF_SIZE) {
    *data = tx_pkt_buf[offset];
  } else {
    *data = enc28j60_packet_rx_byte();    
  }
  offset ++;
}

static void tx_end(u16 pkt_size)
{
  // reset size of packet
  tx_pkt_size = 0;
  enc28j60_packet_rx_byte_end();
  enc28j60_packet_rx_end();
}

// ----- function table -----

static pb_proto_funcs_t funcs = {
  .send_begin = rx_begin,
  .send_data = rx_data,
  .send_end = rx_end,
  
  .recv_begin = tx_begin,
  .recv_data = tx_data,
  .recv_end = tx_end
};

// ----- functions -----

void pb_io_init(void)
{
  pb_proto_init(&funcs);
}

static void uart_send_prefix_pbp(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("pbp(rx): "));
}

static void uart_send_prefix_eth(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("eth(TX): "));
}

//#define DEBUG

static u08 filter_arp_pkt(void)
{
  u08 *arp_buf = rx_pkt_buf + ETH_HDR_SIZE;
  u16 arp_size = rx_pkt_size;
  
  // make sure its an IPv4 ARP packet
  if(!arp_is_ipv4(arp_buf, arp_size)) {
    uart_send_prefix_pbp();
    uart_send_pstring(PSTR("ARP: type?"));
    uart_send_crlf();
    return 0;
  }

  // ARP request should now be something like this:
  // src_mac = Amiga MAC
  // src_ip = Amiga IP
  const u08 *pkt_src_mac = eth_get_src_mac(rx_pkt_buf);
  const u08 *arp_src_mac = arp_get_src_mac(arp_buf);
  
  // pkt src and arp src must be the same
  if(!net_compare_mac(pkt_src_mac, arp_src_mac) && !net_compare_zero_mac(pkt_src_mac)) {
    uart_send_prefix_pbp();
    uart_send_pstring(PSTR("ARP: pkt!=src mac!"));
    uart_send_crlf();
    return 0;
  }
  return 1;
}

static void send(void)
{
  // dump eth packet
  if(param.dump_dirs & DUMP_DIR_ETH_TX) {
    uart_send_prefix_eth();
    dump_line(rx_pkt_buf, rx_pkt_size);
    uart_send_crlf();
  }

  // wait for tx is possible
  enc28j60_packet_tx_prepare();

  // finally send ethernet packet
  enc28j60_packet_tx_send(rx_pkt_size);
}

static void handle_magic_online(void)
{
  net_copy_mac(eth_get_src_mac(rx_pkt_buf), sana_mac);
  const u08 *ver = eth_get_tgt_mac(rx_pkt_buf);
  sana_version[0] = ver[0];
  sana_version[1] = ver[1];
  
  // always show online state
  uart_send_prefix_pbp();
  uart_send_pstring(PSTR("online: mac="));
  net_dump_mac(sana_mac);
  uart_send_pstring(PSTR(" ver="));
  uart_send_hex_byte(sana_version[0]);
  uart_send('.');
  uart_send_hex_byte(sana_version[1]);
  uart_send_crlf();
  
  /* does versions match? */
  if((sana_version[0] == VERSION_MAJ) && (sana_version[1] == VERSION_MIN)) {  
    sana_online = 1;
  } else {
    uart_send_pstring(PSTR("VERSION MISMATCH! please update firmware."));
    uart_send_crlf();
    sana_online = 0;
  }
}

static void handle_magic_offline(void)
{
  uart_send_prefix_pbp();
  uart_send_pstring(PSTR("offline"));
  uart_send_crlf();
  
  sana_online = 0;
}

#define MAGIC_ONLINE 0xffff
#define MAGIC_OFFLINE 0xfffe

static void handle_pb_send(u08 eth_online)
{
  u08 dump_it = param.dump_dirs & DUMP_DIR_PLIP_RX;  
  
  // got a valid packet from plip -> check type
  u16 type = eth_get_pkt_type(rx_pkt_buf);

  // dump packet?
  if(dump_it) {
    uart_send_prefix_pbp();
    dump_line(rx_pkt_buf, rx_pkt_size);
    uart_send_crlf();
  }

  // flag if packet will be sent via ethernet
  u08 send_it = 0;
  
  // ----- magic packets from SANA driver? -----
  // magic online packet?
  if(type == MAGIC_ONLINE) {
    handle_magic_online();
  }
  // magic offline packet?
  else if(type == MAGIC_OFFLINE) {
    handle_magic_offline();
  }
  else {
    // ----- packet filtering? -----
    if(param.filter_plip) {
      if(type == ETH_TYPE_IPV4) {
        send_it = 1;
      }
      // handle ARP packet
      else if(type == ETH_TYPE_ARP) {
        send_it = filter_arp_pkt();
      }
      // unknown packet type
      else {
        uart_send_prefix_pbp();
        uart_send_pstring(PSTR("type? "));
        uart_send_hex_word(type);
        uart_send_crlf();
        send_it = 0;
      }
    } else {
      send_it = 1;
    }
  }
  
  // send to ethernet
  if(send_it) {
    stats.rx_cnt ++;
    stats.rx_bytes += rx_pkt_size;
    
    if(eth_online) {
      send();
    } else {
      // no ethernet online -> we have to drop packet
      uart_send_prefix_eth();
      uart_send_pstring(PSTR("Offline -> DROP!"));
      uart_send_crlf();
      stats.rx_drop ++;
    }      
  } else {
    stats.rx_filter ++;
  }    
}

extern u32 req_time;

void pb_io_worker(u08 plip_state, u08 eth_online)
{
  // call protocol handler (low level transmit)
  u08 cmd;
  u16 size;
  u32 start = time_stamp;
  u08 status = pb_proto_handle(&cmd, &size);
  u32 end = time_stamp;
  
  // nothing done... return
  if(status == PBPROTO_STATUS_IDLE) {
    return;
  }
  else if(status == PBPROTO_STATUS_OK) {
    // io is ok
    if(param.dump_plip) {
      dump_pb_cmd(cmd, status, size, end - start, start - req_time);
    }
    // log command
    if(param.log_all) {
      log_add(start, end-start, cmd, status, size);
    }
    
    // do we have a packet received?
    if(cmd == PBPROTO_CMD_SEND) {
      handle_pb_send(eth_online);
    }
  }
  else {
    u32 delta = end - start;
    u32 latency = start - req_time;
    
    // dump error
    dump_pb_cmd(cmd, status, size, delta, latency);
    
    // store error in log
    log_add(start, delta, cmd, status, size);
  }
}
