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
#include "net/icmp.h"

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
    pkt_buf[offset] = *data;
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

static void handle_ip_pkt(u08 eth_online)
{
  if(param.show_ip) {
    dump_ip_pkt(pkt_buf + ETH_HDR_SIZE, pkt.size - ETH_HDR_SIZE, uart_send_prefix);
  }
  
  if(eth_online) {
    // simply send packet to ethernet
    eth_tx_send(ETH_TYPE_IPV4, pkt.size);
  } else {
    // no ethernet online -> we have to drop packet
    uart_send_prefix();
    uart_send_pstring(PSTR("IP: DROP!"));
    uart_send_crlf();
  }
}

static void handle_arp_pkt(u08 eth_online)
{
  u08 *arp_buf = pkt_buf + ETH_HDR_SIZE;
  u16 arp_size = pkt.size;
  
  // make sure its an IPv4 ARP packet
  if(!arp_is_ipv4(arp_buf, arp_size)) {
    uart_send_prefix();
    uart_send_pstring(PSTR("ARP: type?"));
    return;
  }

  // show arp packet
  if(param.show_arp) {
    dump_arp_pkt(arp_buf, uart_send_prefix);
  }

  // ARP request should now be something like this:
  // src_mac = Amiga MAC
  // src_ip = Amiga IP
  const u08 *pkt_src_mac = eth_get_src_mac(pkt_buf);
  const u08 *arp_src_mac = arp_get_src_mac(arp_buf);
  
  // pkt src and arp src must be the same
  if(!net_compare_mac(pkt_src_mac, arp_src_mac)) {
    uart_send_prefix();
    uart_send_pstring(PSTR("ARP: pkt!=src mac!"));
    uart_send_crlf();
    return;
  }
    
  // send ARP packet to ethernet
  if(eth_online) {
    eth_tx_send(ETH_TYPE_ARP, pkt.size);
  } else {
    // no ethernet online -> we have to drop packet
    uart_send_prefix();
    uart_send_pstring(PSTR("ARP: DROP!"));
    uart_send_crlf();
  }
}

void plip_rx_worker(u08 plip_state, u08 eth_online)
{
  // do we have a PLIP packet waiting?
  if(plip_can_recv() == PLIP_STATUS_OK) {
    // receive PLIP packet and store it in eth chip tx buffer
    // also keep a copy in our local pkt_buf (up to max size)
    u08 status = plip_recv(&pkt);
    if(status == PLIP_STATUS_OK) {

      if(param.show_pkt) {
        dump_eth_pkt(pkt_buf, pkt.size, uart_send_prefix);
      }

      // got a valid packet from plip -> check type
      u16 type = eth_get_pkt_type(pkt_buf);
      // handle IP packet
      if(type == ETH_TYPE_IPV4) {
        handle_ip_pkt(eth_online);
      }
      // handle ARP packet
      else if(type == ETH_TYPE_ARP) {
        handle_arp_pkt(eth_online);
      }
      // unknown packet type
      else {
        uart_send_prefix();
        uart_send_pstring(PSTR("type? "));
        uart_send_hex_word_crlf(type);
      }
    } else {
      // report receiption error
      uart_send_prefix();
      uart_send_pstring(PSTR("recv? "));
      uart_send_hex_byte_crlf(status);
    }
  }
}
