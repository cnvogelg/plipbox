/*
 * plip_rx.c: handle incoming plip packets
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

#include "plip_state.h"
#include "pkt_buf.h"
#include "enc28j60.h"
#include "eth_tx.h"
#include "plip.h"
#include "plip_tx.h"
#include "uart.h"
#include "uartutil.h"
#include "param.h"
#include "dump.h"
#include "timer.h"
#include "stats.h"

static u08 offset;

static u08 begin_rx(plip_packet_t *pkt)
{
  // start writing packet with eth header
  offset = 0;
  enc28j60_packet_tx_begin_range(0);
  
  return PLIP_STATUS_OK;
}

static u08 transfer_rx(u08 *data)
{
  // clone packet to our packet buffer
  if(offset < PKT_BUF_SIZE) {
    rx_pkt_buf[offset] = *data;
    offset ++;
  }
  
  // always copy to TX buffer of enc28j60
  enc28j60_packet_tx_byte(*data);
  
  return PLIP_STATUS_OK;
}

static u08 end_rx(plip_packet_t *pkt)
{
  // end range in enc28j60 buffer
  enc28j60_packet_tx_end_range();

  return PLIP_STATUS_OK;
}

void plip_rx_init(void)
{
  plip_recv_init(begin_rx, transfer_rx, end_rx);
}

static void uart_send_prefix(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("plip(rx): "));
}

//#define DEBUG

static u08 filter_arp_pkt(void)
{
  u08 *arp_buf = rx_pkt_buf + ETH_HDR_SIZE;
  u16 arp_size = rx_pkt.size;
  
  // make sure its an IPv4 ARP packet
  if(!arp_is_ipv4(arp_buf, arp_size)) {
    uart_send_prefix();
    uart_send_pstring(PSTR("ARP: type?"));
    return 0;
  }

  // ARP request should now be something like this:
  // src_mac = Amiga MAC
  // src_ip = Amiga IP
  const u08 *pkt_src_mac = eth_get_src_mac(rx_pkt_buf);
  const u08 *arp_src_mac = arp_get_src_mac(arp_buf);
  
  // pkt src and arp src must be the same
  if(!net_compare_mac(pkt_src_mac, arp_src_mac) && !net_compare_zero_mac(pkt_src_mac)) {
    uart_send_prefix();
    uart_send_pstring(PSTR("ARP: pkt!=src mac!"));
    uart_send_crlf();
    return 0;
  }
  return 1;
}

static u08 rx_postponed;

static void send(void)
{
  // simply send packet to ethernet
  dump_latency_data.tx_enter = time_stamp;
  eth_tx_send(rx_pkt.size);
  dump_latency_data.tx_leave = time_stamp;
  
  if(param.dump_latency) {
    uart_send_prefix();
    dump_latency();
    uart_send_crlf();
  }
}

void plip_rx_worker(u08 plip_state, u08 eth_online)
{
  // if last packet was postponed then send it now
  if(rx_postponed) {
    send();
    rx_postponed = 0;
  }
  
  u08 dump_it = param.dump_dirs & DUMP_DIR_PLIP_RX;
  
  // do we have a PLIP packet waiting?
  if(plip_can_recv() == PLIP_STATUS_OK) {
    
    // receive PLIP packet and store it in eth chip tx buffer
    // also keep a copy in our local pkt_buf (up to max size)
    dump_latency_data.rx_enter = time_stamp;
    u08 status = plip_recv(&rx_pkt);
    dump_latency_data.rx_leave = time_stamp;
        
    // packet receive was ok
    if(status == PLIP_STATUS_OK) {      
      // got a valid packet from plip -> check type
      u16 type = eth_get_pkt_type(rx_pkt_buf);

      // dump packet?
      if(dump_it) {
        uart_send_prefix();
        dump_line(rx_pkt_buf, rx_pkt.size);
        if(param.dump_plip) {
          dump_plip();
        }
        uart_send_crlf();
      }

      // filter packet
      u08 send_it = 0;
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
          uart_send_prefix();
          uart_send_pstring(PSTR("type? "));
          uart_send_hex_word(type);
          uart_send_crlf();
          send_it = 0;
        }
      } else {
        send_it = 1;
      }
      
      // send to ethernet
      if(send_it) {
        stats.rx_cnt ++;
        stats.rx_bytes += rx_pkt.size;
        
        if(eth_online) {
          if(plip_tx_get_retries() == 0) {
            send();
          } else {
            // is a tx packet currently postponed? -> delay sending my packet
            rx_postponed = 1;
          }
        } else {
          // no ethernet online -> we have to drop packet
          uart_send_prefix();
          uart_send_pstring(PSTR("Offline -> DROP!"));
          uart_send_crlf();
          stats.rx_drop ++;
        }      
      } else {
        stats.rx_filter ++;
      }
      
    } else {
      // account stats
      stats.rx_err ++;
      stats.last_rx_err = status;
      
      // report receiption error
      uart_send_prefix();
      uart_send_pstring(PSTR("recv? "));
      uart_send_hex_byte(status);
      uart_send_crlf();
    }
  }
}
