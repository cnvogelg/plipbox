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

void arp_dump(const u08 *buf)
{
  uart_send_string("arp:src_mac=");
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
