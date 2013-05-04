/*
 * eth_rx.c: handle eth receiption and plip tx
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

#include <avr/delay.h>

#include "eth_rx.h"

#include "net/icmp.h"
#include "net/arp.h"
#include "net/eth.h"
#include "net/net.h"
#include "net/ip.h"

#include "net/udp.h"
#include "net/eth.h"
#include "net/net.h"

#include "enc28j60.h"
#include "pkt_buf.h"
#include "uartutil.h"
#include "uart.h"
#include "plip_tx.h"
#include "timer.h"
#include "eth_state.h"
#include "dump.h"
#include "param.h"

void eth_rx_init(void)
{
}

static void uart_send_prefix(void)
{
  uart_send_pstring(PSTR(" eth(rx): "));
}

static u08 filter_packet(const u08 *eth_buf)
{
  // drop unknown types
  u16 type = eth_get_pkt_type(eth_buf);
  if((type != ETH_TYPE_ARP) && (type != ETH_TYPE_IPV4)) {
    if(param.show_drop) {
      uart_send_prefix();
      uart_send_pstring(PSTR("type? "));
      uart_send_hex_word_crlf(type);
    }
    return 0;
  }
  
  // tgt mac: either my mac or broadcast
  const u08 *tgt_addr = eth_get_tgt_mac(eth_buf);
  if((!net_compare_mac(param.mac_addr,tgt_addr)) && (!net_compare_bcast_mac(tgt_addr))) {
    if(param.show_drop) {
      uart_send_prefix();
      uart_send_pstring(PSTR("mac? "));
      net_dump_mac(tgt_addr);
      uart_send_crlf();
    }
  }
  
  // ok. pass packet
  return 1;
}

void eth_rx_worker(u08 eth_state, u08 plip_online)
{
  if(eth_state == ETH_STATE_LINK_DOWN) {
    return;
  }
  
  // get next packet
  u16 len = enc28j60_packet_rx_begin();
  if(len > 0) {
    // first read only the ethernet header into our buffer
    enc28j60_packet_rx_blk(pkt_buf, ETH_HDR_SIZE);
    
    // dump incoming packet
    if(param.show_pkt) {
      dump_eth_pkt(pkt_buf, len, uart_send_prefix);
    }

    // depending on type fetch more from buffer
    u16 mem_len = ETH_HDR_SIZE;
    u08 *buf = pkt_buf + ETH_HDR_SIZE;
    u16 type = eth_get_pkt_type(pkt_buf);
    if(type == ETH_TYPE_ARP) {
      // ARP
      mem_len += ARP_SIZE;
      enc28j60_packet_rx_blk(buf, ARP_SIZE);
      // dump
      if(param.show_arp) {
        dump_arp_pkt(buf,uart_send_prefix);
      }
    } else if(type == ETH_TYPE_IPV4) {
      // IPv4
      mem_len += IP_MIN_HDR_SIZE;
      enc28j60_packet_rx_blk(buf, IP_MIN_HDR_SIZE);
      // dump
      if(param.show_ip) {
        dump_ip_pkt(buf,len - ETH_HDR_SIZE,uart_send_prefix);
      }
    }
    
    // what to do with the eth packet?
    if(plip_online) {    
      // if it does pass filter then send it via plip
      if(filter_packet(pkt_buf)) {
        // send full pkt via plip
        plip_tx_send(0, mem_len, len);
      } else {
        if(param.show_drop) {
          uart_send_prefix();
          uart_send_pstring(PSTR("drop: filter!")),
          uart_send_crlf();
        }
      }
    } else {
      if(param.show_drop) {
        uart_send_prefix();
        uart_send_pstring(PSTR("drop: offline!"));
        uart_send_crlf();
      }
    }
  
    // finish packet read
    enc28j60_packet_rx_end();
  }
}
