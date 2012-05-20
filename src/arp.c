/*
 * arp.c - handle ARP protocol
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plip2slip.
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

#include "arp.h"
#include "net.h"
#include "eth.h"
#include "uartutil.h"
#include "enc28j60.h"

#define ARP_OFF_HW_TYPE   0
#define ARP_OFF_PROT_TYPE 2
#define ARP_OFF_HW_SIZE   4
#define ARP_OFF_PROT_SIZE 5
#define ARP_OFF_OP        6
#define ARP_OFF_SRC_MAC   8
#define ARP_OFF_SRC_IP    14
#define ARP_OFF_TGT_MAC   18
#define ARP_OFF_TGT_IP    24

#define ARP_SIZE 28

#define ARP_REQUEST   1
#define ARP_REPLY     2

   /* debug */
#define DUMP_ARP

   /* ARP cache */
static u08 gw_mac[6];


u08 arp_is_ipv4(const u08 *buf, u16 len)
{
  if(len < ARP_SIZE) {
    return 0;
  }
  
  u16 hw_type = net_get_word(buf + ARP_OFF_HW_TYPE);
  u16 pt_type = net_get_word(buf + ARP_OFF_PROT_TYPE);
  u08 hw_size = buf[ARP_OFF_HW_SIZE];
  u08 pt_size = buf[ARP_OFF_PROT_SIZE];
  
  return (hw_type == 1) && (pt_type == 0x800) && (hw_size == 6) && (pt_size == 4);
}

u16 arp_get_op(const u08 *buf)
{
  return net_get_word(buf + ARP_OFF_OP);
}

u08 arp_is_req_for_me(const u08 *buf)
{
  return (arp_get_op(buf) == ARP_REQUEST) && net_compare_my_ip(buf + ARP_OFF_TGT_IP);
}

u08 arp_is_reply_for_me(const u08 *buf)
{
  return (arp_get_op(buf) == ARP_REPLY) && net_compare_my_ip(buf + ARP_OFF_TGT_IP);
}

void arp_make_reply(u08 *buf)
{
  net_put_word(buf + ARP_OFF_OP, ARP_REPLY);
  // copy old src to new tgt
  net_copy_mac(buf + ARP_OFF_SRC_MAC, buf + ARP_OFF_TGT_MAC);
  net_copy_ip(buf + ARP_OFF_SRC_IP, buf + ARP_OFF_TGT_IP);
  // set my own mac/ip as src
  net_copy_my_mac(buf + ARP_OFF_SRC_MAC);
  net_copy_my_ip(buf + ARP_OFF_SRC_IP);
}

u16 arp_make_request(u08 *buf, const u08 ip[4])
{
  // IPv4 ARP
  net_put_word(buf + ARP_OFF_HW_TYPE, 1);
  net_put_word(buf + ARP_OFF_PROT_TYPE, 0x800);
  buf[ARP_OFF_HW_SIZE] = 6;
  buf[ARP_OFF_PROT_SIZE] = 4;
  // request
  net_put_word(buf + ARP_OFF_OP, ARP_REQUEST);
  // me being the source
  net_copy_my_mac(buf + ARP_OFF_SRC_MAC);
  net_copy_my_ip(buf + ARP_OFF_SRC_IP);
  // tgt being the request host
  net_copy_zero_mac(buf + ARP_OFF_TGT_MAC);
  net_copy_ip(ip, buf + ARP_OFF_TGT_IP);
  return ARP_SIZE;
}

void arp_dump(const u08 *buf)
{
  uart_send_string("ARP:src_mac=");
  net_dump_mac(buf + ARP_OFF_SRC_MAC);
  uart_send_string(",src_ip=");
  net_dump_ip(buf + ARP_OFF_SRC_IP);
  uart_send_string(",tgt_mac=");
  net_dump_mac(buf + ARP_OFF_TGT_MAC);
  uart_send_string(",tgt_ip=");
  net_dump_ip(buf + ARP_OFF_TGT_IP);
  uart_send_string(",op=");
  u16 op = arp_get_op(buf);
  uart_send_hex_word_spc(op);
}

void arp_init(u08 *ethbuf, u16 maxlen)
{
  // build a request for gw ip
  eth_make_to_any(ethbuf, ETH_TYPE_ARP);
  u16 len = arp_make_request(ethbuf + ETH_HDR_SIZE, net_get_gateway());
  len += ETH_HDR_SIZE;

#ifdef DUMP_ARP
  uart_send_string("arp:req_gw ");
  net_dump_ip(net_get_gateway());
  uart_send_crlf();
  uart_send_hex_word_spc(len);
  eth_dump(ethbuf);
  uart_send_crlf();
  arp_dump(ethbuf + ETH_HDR_SIZE);
  uart_send_crlf();
#endif
  
  enc28j60_packet_send(ethbuf, len);
}

u08 arp_handle_packet(u08 *ethbuf, u16 ethlen)
{
  /* return if its no ARP packet */
  if(!eth_is_arp_pkt(ethbuf)) {
    return 0;
  }
  
  u08 *buf = ethbuf + ETH_HDR_SIZE;
  u16 len  = ethlen - ETH_HDR_SIZE;
  
  /* return if its no IPv4 ARP packet */
  if(!arp_is_ipv4(buf, len)) {
    return 0;
  }
  
#ifdef DUMP_ARP
  arp_dump(buf);
#endif
  
  /* is someone asking for our MAC? */
  if(arp_is_req_for_me(buf)) {
#ifdef DUMP_ARP
    uart_send_string("ME! ");
#endif
    /* send a reply with my MAC+IP */
    eth_make_reply(ethbuf);
    arp_make_reply(buf);
    enc28j60_packet_send(ethbuf, ethlen);
  }
  
  /* is it a reply for a request of me? */
  else if(arp_is_reply_for_me(buf)) {
#ifdef DUMP_ARP
    uart_send_string("REPLY!");
#endif
    /* we got a reply for the gateway MAC -> keep in cache */
    if(net_compare_ip(buf + ARP_OFF_SRC_IP, net_get_gateway())) {
      net_copy_mac(buf + ARP_OFF_SRC_MAC, gw_mac);
#ifdef DUMP_ARP
      uart_send_crlf();
      uart_send_string("arp:rep_gw ");
      net_dump_ip(net_get_gateway());
      uart_send_spc();
      net_dump_mac(gw_mac);
#endif
    }
    
  }
  
#ifdef DUMP_ARP
  uart_send_crlf();      
#endif
  
  return 1;
}

