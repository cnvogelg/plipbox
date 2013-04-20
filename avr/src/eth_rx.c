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

#ifdef HAVE_NAT
#include "net/arp_cache.h"
#include "net/dhcp_client.h"
#endif

#include "net/udp.h"
#include "net/eth.h"
#include "net/net.h"

#include "enc28j60.h"
#include "pkt_buf.h"
#include "uartutil.h"
#include "uart.h"
#include "ping.h"
#include "plip_tx.h"
#include "timer.h"
#include "eth_state.h"
#include "debug.h"

void eth_rx_init(void)
{
#ifdef HAVE_NAT
  // prepare cache
  arp_cache_init();
#endif
}

static void uart_send_prefix(void)
{
  uart_send_pstring(PSTR(" eth(rx): "));
}

static u08 filter_packet(plip_packet_t *pkt)
{
  // drop unknown types
  u16 type = pkt->type;
  if((type != ETH_TYPE_ARP) && (type != ETH_TYPE_IPV4)) {
    uart_send_prefix();
    uart_send_pstring(PSTR("type? "));
    uart_send_hex_word_crlf(type);
    return 0;
  }
  
  // tgt mac: either my mac or broadcast
  if((!net_compare_my_mac(pkt->dst_addr)) && (!net_compare_bcast_mac(pkt->dst_addr))) {
    uart_send_prefix();
    uart_send_pstring(PSTR("mac? "));
    net_dump_mac(pkt->dst_addr);
    uart_send_crlf();
  }
  
  // ok. pass packet
  return 1;
}

void eth_rx_worker(u08 eth_state, u08 plip_online)
{
  if(eth_state == ETH_STATE_OFFLINE) {
    return;
  }
  
  // get next packet
  u16 len = enc28j60_packet_rx_begin();
  if(len > 0) {
    // first read only the ethernet header into our buffer
    enc28j60_packet_rx_blk(pkt_buf, ETH_HDR_SIZE);
  
    // copy src/tgt mac and type from eth frame
    net_copy_mac(eth_get_tgt_mac(pkt_buf), pkt.dst_addr);
    net_copy_mac(eth_get_src_mac(pkt_buf), pkt.src_addr);
    pkt.type = eth_get_pkt_type(pkt_buf);
    
    if(plip_online) {    
      // if it does pass filter then send it via plip
      if(filter_packet(&pkt)) {
        // send pkt via plip
        u16 ip_len = len - ETH_HDR_SIZE;
        plip_tx_send(0, 0, ip_len);
      } else {
#ifdef DEBUG
        debug_dump_plip_pkt(&pkt, uart_send_prefix);
#endif
        uart_send_prefix();
        uart_send_pstring(PSTR("drop!")),
        uart_send_crlf();
      }
    } else {
      uart_send_prefix();
      uart_send_pstring(PSTR("offline!"));
      uart_send_crlf();
    }
  
    // finish packet read
    enc28j60_packet_rx_end();
  }
}
