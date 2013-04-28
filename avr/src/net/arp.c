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
