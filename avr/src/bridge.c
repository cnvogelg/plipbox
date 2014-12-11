/*
 * bridge.c: bridge packets from plip to eth and back
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
#include "pio.h"
#include "pb_proto.h"
#include "uart.h"
#include "uartutil.h"
#include "param.h"
#include "dump.h"
#include "timer.h"
#include "stats.h"
#include "util.h"
#include "bridge.h"
#include "main.h"
#include "cmd.h"

static u08 req_pending;
static u32 req_ts;
static u32 req_delta;

#if 0
static u16 offset;
static u16 size;
static u08 send_magic;

// SANA driver parameters sent in magic packet
u08 sana_online;
u08 sana_mac[6];
u08 sana_version[2];

// ----- functions -----

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

  // finally send ethernet packet
  pktio_tx_send(rx_pkt_size);
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
  
  /* neet to reconfigure eth? */
  if(!net_compare_mac(sana_mac, param.mac_addr)) {
    net_copy_mac(sana_mac, param.mac_addr);
    param_save(); /* save changed mac to eeprom */
    eth_state_configure(); /* configure eth to set new mac */
  }

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

// packet arrived from Amiga. now sent via Ethernet
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
    if(eth_online) {
      stats.tx_cnt ++;
      stats.tx_bytes += rx_pkt_size;    
      send();
    } else {
      // no ethernet online -> we have to drop packet
      uart_send_prefix_eth();
      uart_send_pstring(PSTR("Offline -> DROP!"));
      uart_send_crlf();
      stats.tx_drop ++;
      pktio_tx_reject();
    }      
  } else {
    stats.tx_filter ++;
    pktio_tx_reject();
  }    
}

void pb_io_send_magic(u16 type, u08 extra_size)
{
  // set tx packet size so no ethernet frames are written in packet buffer
  tx_pkt_size = 14 + extra_size;
  send_magic = 1;

  // fill ethernet header
  u08 *ptr = tx_pkt_buf;
  net_copy_bcast_mac(ptr);
  ptr += 6;
  net_copy_mac(param.mac_addr, ptr);
  ptr += 6;
  net_put_word(ptr, type);

  // now request receive from Amiga
  pb_proto_request_recv();

  // report
  uart_send_prefix_pbp();
  uart_send_pstring(PSTR("send magic "));
  uart_send_hex_word(type);
  uart_send_crlf();
}

extern u32 req_time;
#endif

// ----- packet callbacks -----

static u08 fill_pkt(u08 *buf, u16 max_size, u16 *size)
{
  // confirm request
  req_delta = time_stamp - req_ts;
  req_pending = 0;

  // receive a packet
  pio_recv(buf, max_size, size);

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("fill: n="));
  uart_send_hex_word(*size);
  uart_send_crlf();

#if 0
  if(got_size >= 14) {
    dump_eth_pkt(buf,got_size);
    uart_send_crlf();
  }
#endif

  return PBPROTO_STATUS_OK;  
}

static u08 proc_pkt(const u08 *buf, u16 size)
{
  pio_send(buf, size);

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("proc: n="));
  uart_send_hex_word(size);
  uart_send_crlf();  

#if 0
  dump_eth_pkt(buf,size);
  uart_send_crlf();
#endif

  return PBPROTO_STATUS_OK;
}

// ----- function table -----

static pb_proto_funcs_t funcs = {
  .fill_pkt = fill_pkt,
  .proc_pkt = proc_pkt
};

// ---------- loop ----------

static void bridge_worker(void)
{
#if 0
  // check pio
  if(!req_pending && (pio_has_recv() > 0)) {
    pb_proto_request_recv();
    req_pending = 1;
    req_ts = time_stamp;
  }

  // call protocol handler (low level transmit)
  u08 cmd;
  u16 size;
  u16 delta;
  u08 status = pb_proto_handle(&cmd, &size, &delta);
  u16 rate = timer_hw_calc_rate_kbs(size, delta);
  u08 is_tx = (cmd == PBPROTO_CMD_SEND) || (cmd == PBPROTO_CMD_SEND_BURST);
  u08 stats_id = is_tx ? STATS_ID_PB_TX : STATS_ID_PB_RX;

  // nothing done... return
  if(status == PBPROTO_STATUS_IDLE) {
    return; // inactive
  }

  // ok!
  if(status == PBPROTO_STATUS_OK) {
    // account data
    stats_update_ok(stats_id, size, rate);
    // always dump proto
    if(param.dump_plip) {
      dump_pb_cmd(cmd, status, size, delta, rate, req_delta);
    }
  }
  // pb proto failed with an error
  else {
    // dump error
    dump_pb_cmd(cmd, status, size, delta, rate, req_delta);
    // account data
    stats_get(stats_id)->err++;
  }
#endif
}

u08 bridge_loop(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[BRIDGE] on\r\n"));

  stats_reset();
  pb_proto_init(&funcs, pkt_buf, PKT_BUF_SIZE);
  pio_init(param.mac_addr, PIO_INIT_BROAD_CAST);

  // main loop
  u08 reset = 0;
  while(run_mode == RUN_MODE_BRIDGE) {
    reset = !cmd_worker();
    if(reset) {
      break;
    }

    bridge_worker();
  }

  stats_dump_all();

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[BRIDGE] off\r\n"));
  return reset;
}
