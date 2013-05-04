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
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR(" eth(rx): "));
}

static u08 filter_packet(const u08 *eth_buf, u08 dump_flag)
{
  // drop unknown types
  u16 type = eth_get_pkt_type(eth_buf);
  if((type != ETH_TYPE_ARP) && (type != ETH_TYPE_IPV4)) {
    if(dump_flag) {
      uart_send_pstring(PSTR("type? "));
    }
    return 0;
  }
  
  // tgt mac: either my mac or broadcast
  const u08 *tgt_addr = eth_get_tgt_mac(eth_buf);
  if((!net_compare_mac(param.mac_addr,tgt_addr)) && (!net_compare_bcast_mac(tgt_addr))) {
    if(dump_flag) {
      uart_send_pstring(PSTR("mac? "));
    }
    return 0;
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
    // try to fill packet buffer
    u16 mem_len = len;
    if(mem_len > PKT_BUF_SIZE) {
      mem_len = PKT_BUF_SIZE;
    }
    
    // pre-fetch via SPI into packet buffer
    enc28j60_packet_rx_blk(pkt_buf, mem_len);
    
    // dump incoming packet
    u08 dump_flag = param.dump_dirs & DUMP_DIR_ETH_RX;
    if(dump_flag) {
      uart_send_prefix();
      dump_line(pkt_buf, len);
    }
    
    // what to do with the eth packet?
    if(plip_online) {    
      // if it does pass filter then send it via plip
      if(filter_packet(pkt_buf, dump_flag)) {
        // finish dump line first
        if(dump_flag) {
          uart_send_crlf();
        }
        
        // send full pkt via plip
        plip_tx_send(0, mem_len, len);
      } else {
        if(dump_flag) {
          uart_send_pstring(PSTR("<drop:Filter>")),
          uart_send_crlf();
        }
      }
    } else {
      if(dump_flag) {
        uart_send_pstring(PSTR("<drop:Offline>"));
        uart_send_crlf();
      }
    }
  
    // finish packet read
    enc28j60_packet_rx_end();
  }
}
