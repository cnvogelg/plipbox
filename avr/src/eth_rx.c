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
#include "net/ip.h"
#include "net/udp.h"

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

static u08 filter_broadcast_ipv4(const u08 *eth_buf, u08 dump_flag)
{
  const u08 *ip_buf = eth_buf + ETH_HDR_SIZE;
  const u08 *tgt_ip = ip_get_tgt_ip(ip_buf);
  u08 is_ip_bcast = net_compare_bcast_ip(tgt_ip);
  // accept eth broadcast only if ip is also a broadcast (255.255.255.255)
  if(!is_ip_bcast) {
    if(dump_flag) {
      uart_send_pstring(PSTR("bcast ip? "));
    }
    return 0;
  }
  // make sure its a udp
  u08 ip_proto = ip_get_protocol(ip_buf);
  if(ip_proto != IP_PROTOCOL_UDP) {
    if(dump_flag) {
      uart_send_pstring(PSTR("bcast no udp? "));
    }
    return 0;
  }
  // make sure its a bootp udp
  const u08 *udp_buf = ip_buf + ip_get_hdr_length(ip_buf);
  u16 src_port = udp_get_src_port(udp_buf);
  u16 tgt_port = udp_get_tgt_port(udp_buf);
  u08 is_bootp = ((src_port == 67) && (tgt_port == 68)) ||
    ((src_port == 68) && (tgt_port == 67));
  if(!is_bootp) {
    if(dump_flag) {
      uart_send_pstring(PSTR("no bootp? "));
    }
    return 0;
  }
  return 1;
}

static u08 filter_packet(const u08 *eth_buf, u08 dump_flag)
{
  // drop unknown types
  u16 type = eth_get_pkt_type(eth_buf);
  u08 is_ipv4 = (type == ETH_TYPE_IPV4);
  u08 is_arp = (type == ETH_TYPE_ARP);
  if(!is_ipv4 && !is_arp) {
    if(dump_flag) {
      uart_send_pstring(PSTR("type? "));
    }
    return 0;
  }
  
  // tgt mac: either my mac or broadcast
  const u08 *tgt_addr = eth_get_tgt_mac(eth_buf);
  u08 is_bcast = net_compare_bcast_mac(tgt_addr);
  u08 is_mine  = net_compare_mac(param.mac_addr,tgt_addr);
  if(!is_mine && !is_bcast) {
    if(dump_flag) {
      uart_send_pstring(PSTR("mac? "));
    }
    return 0;
  }
  
  // check broadcast
  if(is_bcast) {
    if(is_ipv4) {
      return filter_broadcast_ipv4(eth_buf, dump_flag);
    }
  }
  
  // ok. pass packet
  return 1;
}

void eth_rx_worker(u08 eth_state, u08 plip_online)
{
  // first try to send postponed
  if(plip_tx_get_retries() > 0) {
    u08 status = plip_tx_send_retry();
    if(status != PLIP_STATUS_OK) {
      return;
    }
  }
  
  if(eth_state == ETH_STATE_LINK_DOWN) {
    return;
  }

  // get next packet
  dump_latency_data.rx_enter = time_stamp;
  u16 len = enc28j60_packet_rx_begin();
  if(len > 0) {    
    // try to fill packet buffer
    u16 mem_len = len;
    if(mem_len > PKT_BUF_SIZE) {
      mem_len = PKT_BUF_SIZE;
    }
    
    // pre-fetch via SPI into packet buffer (-> tx plip bufer)
    enc28j60_packet_rx_blk(tx_pkt_buf, mem_len);
    dump_latency_data.rx_leave = time_stamp;
    
    // dump incoming packet
    u08 dump_flag = param.dump_dirs & DUMP_DIR_ETH_RX;
    if(dump_flag) {
      uart_send_prefix();
      dump_line(tx_pkt_buf, len);
    }
    
    // what to do with the eth packet?
    if(plip_online) {    
      // if it does pass filter then send it via plip
      if(filter_packet(tx_pkt_buf, dump_flag)) {
        // finish dump line first
        if(dump_flag) {
          uart_send_crlf();
        }
        
        // send full pkt via plip
        dump_latency_data.tx_enter = time_stamp;
        plip_tx_send(0, mem_len, len);
        dump_latency_data.tx_leave = time_stamp;
        
        if(param.dump_latency) {
          uart_send_prefix();
          dump_latency();
          uart_send_crlf();
        }
      } else {
        if(dump_flag) {
          uart_send_pstring(PSTR("-> DROP")),
          uart_send_crlf();
        }
      }
    } else {
      if(dump_flag) {
        uart_send_pstring(PSTR("Offline -> DROP"));
        uart_send_crlf();
      }
    }
  
    // finish packet read
    enc28j60_packet_rx_end();
  }
}
