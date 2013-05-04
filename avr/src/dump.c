/*
 * dump.c - helper functions for debugging
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

#include "dump.h"
#include "uartutil.h"
#include "net/net.h"
#include "net/arp.h"
#include "net/eth.h"
#include "net/ip.h"

void dump_eth_pkt(const u08 *eth_buf, u16 size, write_prefix_func_t f)
{
  f();
  uart_send_pstring(PSTR("pkt: src="));
  net_dump_mac(eth_get_src_mac(eth_buf));
  uart_send_pstring(PSTR(" dst="));
  net_dump_mac(eth_get_tgt_mac(eth_buf));
  uart_send_pstring(PSTR(" type="));
  uart_send_hex_word_spc(eth_get_pkt_type(eth_buf));
  uart_send_pstring(PSTR("size="));
  uart_send_hex_word_crlf(size);  
}

void dump_arp_pkt(const u08 *arp_buf, write_prefix_func_t f)
{
  f();
  uart_send_pstring(PSTR("-> ARP: op="));
  u16 op = arp_get_op(arp_buf);
  uart_send_hex_word_spc(op);
  uart_send_crlf();
  
  f();
  uart_send_pstring(PSTR("    src mac="));
  net_dump_mac(arp_get_src_mac(arp_buf));
  uart_send_pstring(PSTR(" ip="));
  net_dump_ip(arp_get_src_ip(arp_buf));
  uart_send_crlf();
  
  f();
  uart_send_pstring(PSTR("    tgt mac="));
  net_dump_mac(arp_get_tgt_mac(arp_buf));
  uart_send_pstring(PSTR(" ip="));
  net_dump_ip(arp_get_tgt_ip(arp_buf));
  uart_send_crlf();
}

void dump_ip_pkt(const u08 *ip_buf, u16 len, write_prefix_func_t f)
{
  f();
  uart_send_pstring(PSTR("-> IP: proto="));
  uart_send_hex_byte_spc(ip_get_protocol(ip_buf));
  uart_send_pstring(PSTR("  len="));
  uart_send_hex_word_spc(ip_get_total_length(ip_buf));
  uart_send_pstring(PSTR("  got="));
  uart_send_hex_word_spc(len);
  uart_send_pstring(PSTR("  src ip="));
  net_dump_ip(ip_get_src_ip(ip_buf));
  uart_send_pstring(PSTR("  tgt ip="));
  net_dump_ip(ip_get_tgt_ip(ip_buf)),
  uart_send_crlf();
}
