/*
 * arp.c - handle ARP protocol
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

#include "arp.h"
#include "net.h"
#include "eth.h"

#include "uartutil.h"

   /* debug */
#define DUMP_ARP

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
  uart_send_pstring(PSTR("ARP:src_mac="));
  net_dump_mac(buf + ARP_OFF_SRC_MAC);
  uart_send_pstring(PSTR(",src_ip="));
  net_dump_ip(buf + ARP_OFF_SRC_IP);
  uart_send_pstring(PSTR(",tgt_mac="));
  net_dump_mac(buf + ARP_OFF_TGT_MAC);
  uart_send_pstring(PSTR(",tgt_ip="));
  net_dump_ip(buf + ARP_OFF_TGT_IP);
  uart_send_pstring(PSTR(",op="));
  u16 op = arp_get_op(buf);
  uart_send_hex_word_spc(op);
}

void arp_send_request(u08 *ethbuf, const u08 *ip, net_tx_packet_func tx_func)
{
  // build a request for gw ip
  eth_make_to_any(ethbuf, ETH_TYPE_ARP);
  u16 len = arp_make_request(ethbuf + ETH_HDR_SIZE, ip);
  len += ETH_HDR_SIZE;

#ifdef DUMP_ARP
  uart_send_pstring(PSTR("arp: requesting MAC for "));
  net_dump_ip(ip);
  uart_send_crlf();
#endif
#ifdef DUMP_DETAIL
  uart_send_hex_word_spc(len);
  eth_dump(ethbuf);
  uart_send_crlf();
  arp_dump(ethbuf + ETH_HDR_SIZE);
  uart_send_crlf();
#endif
  
  tx_func(ethbuf, len);  
}
