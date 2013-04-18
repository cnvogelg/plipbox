/*
 * ping.c - ICMP ping
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

#include "ping.h"

#include "net/net.h"
#include "net/eth.h"
#include "net/ip.h"
#include "net/icmp.h"
#include "net/arp.h"
#include "net/arp_cache.h"

#include "uartutil.h"
#include "pkt_buf.h"
#include "enc28j60.h"
#include "plip_tx.h"

#define DEBUG_PING   
   
u08 ping_eth_send_request(const u08 *ip, u16 id, u16 seq)
{
#ifdef DEBUG_PING
  uart_send_pstring(PSTR("ping: send eth:  "));
  net_dump_ip(ip);
  uart_send_pstring(PSTR(" id="));
  uart_send_hex_word_spc(id);
  uart_send_pstring(PSTR("seq="));
  uart_send_hex_word_spc(seq);
#endif

  const u08 *mac = arp_cache_find_mac(ip);
  if(mac == 0) {
#ifdef DEBUG_PING
    uart_send_pstring(PSTR(" -> no mac!"));
    uart_send_crlf();
#endif
    return 0;
  }
  
#ifdef DEBUG_PING
  uart_send_pstring(PSTR(" -> "));
  net_dump_mac(mac);
  uart_send_crlf();
#endif
  
  eth_make_to_tgt(pkt_buf, ETH_TYPE_IPV4, mac);
  u08 *ip_pkt = pkt_buf + ETH_HDR_SIZE;
  u16 size = icmp_make_ping_request(ip_pkt, net_get_ip(), ip, id, seq);
  enc28j60_packet_tx(pkt_buf, size + ETH_HDR_SIZE);
  
  return 1;
}

u08 ping_plip_send_request(const u08 *ip, u16 id, u16 seq)
{
#ifdef DEBUG_PING
  uart_send_pstring(PSTR("ping: send plip: "));
  net_dump_ip(ip);
  uart_send_pstring(PSTR(" id="));
  uart_send_hex_word_spc(id);
  uart_send_pstring(PSTR("seq="));
  uart_send_hex_word_spc(seq);
  uart_send_crlf();
#endif

  const u08 *my_ip;
#ifdef HAVE_NAT
  my_ip = net_get_p2p_me();
#else
  my_ip = net_get_ip();
#endif  
  u16 size = icmp_make_ping_request(pkt_buf, my_ip, ip, id, seq);
  u08 status = plip_tx_send(0,size,size);
  return (status == PLIP_STATUS_OK);
}

static u08 ping_handle_packet(u08 *ip_buf, u16 ip_len)
{
  uart_send_pstring(PSTR("ping: "));
  if(!icmp_validate_checksum(ip_buf)) {
    uart_send_pstring(PSTR("CHECKSUM!\r\n"));
  } else {
    // incoming request -> generate reply
    if(icmp_is_ping_request(ip_buf)) {
      uart_send_pstring(PSTR("got request: "));
      net_dump_ip(ip_get_src_ip(ip_buf));
      uart_send_pstring(PSTR(" id="));
      uart_send_hex_word_spc(icmp_get_ping_id(ip_buf));
      uart_send_pstring(PSTR("seq="));
      uart_send_hex_word_crlf(icmp_get_ping_seqnum(ip_buf));
      
      // generate reply      
      icmp_ping_request_to_reply(ip_buf);
      return 1;
    }
    // incoming reply -> show!
    else if(icmp_is_ping_reply(ip_buf)) {
      uart_send_pstring(PSTR("got reply: "));
      net_dump_ip(ip_get_src_ip(ip_buf));
      uart_send_pstring(PSTR(" id="));
      uart_send_hex_word_spc(icmp_get_ping_id(ip_buf));
      uart_send_pstring(PSTR("seq="));
      uart_send_hex_word_crlf(icmp_get_ping_seqnum(ip_buf));
    }
    // unknown icmp
    else {
      uart_send_pstring(PSTR("no ping!\r\n"));
    }
  }
  return 0;
}

void ping_eth_handle_packet(u08 *ip_buf, u16 ip_len)
{
  if(ping_handle_packet(ip_buf, ip_len)) {
    // handler created a reply -> send it!
    eth_make_reply(pkt_buf);
    enc28j60_packet_tx(pkt_buf, ip_len + ETH_HDR_SIZE);
  }
}

void ping_plip_handle_packet(u08 *ip_buf, u16 ip_len)
{
  if(ping_handle_packet(ip_buf, ip_len)) {
    plip_tx_send(ETH_HDR_SIZE,ip_len,ip_len);
  }
}

