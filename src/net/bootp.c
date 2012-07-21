/*
 * bootp.c - handle BOOTP messages
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
   
#include <string.h>

#include "bootp.h"
#include "udp.h"
#include "eth.h"
   
#include "uart.h"
#include "uartutil.h"

static u32 xid = 0xdeadbeef;

u08 bootp_begin_pkt(u08 *buf, u08 op)
{
  u08 off = udp_begin_pkt(buf, net_zero_ip, 68, net_ones_ip, 67);
  u08 *ptr = buf + off;
  
  memset(ptr, 0, BOOTP_MIN_SIZE);

  ptr[BOOTP_OFF_OP] = op;
  ptr[BOOTP_OFF_HTYPE] = 0x01;
  ptr[BOOTP_OFF_HLEN] = 0x06;
  net_put_long(ptr + BOOTP_OFF_XID, xid);
  net_copy_my_mac(ptr + BOOTP_OFF_CHADDR);
  
  return off;
}
   
u16 bootp_finish_pkt(u08 *buf, u16 size)
{
  return udp_finish_pkt(buf, size);
}

u08 bootp_begin_swap_pkt(u08 *buf)
{
  // setup IP
  ip_set_src_ip(buf, net_zero_ip);
  ip_set_tgt_ip(buf, net_ones_ip);
  
  // swap UDP ports
  u08 off = ip_get_hdr_length(buf);
  u08 *ptr = buf + off;
  u16 src_port = udp_get_src_port(ptr);
  u16 tgt_port = udp_get_tgt_port(ptr);
  udp_set_src_port(ptr, tgt_port);
  udp_set_tgt_port(ptr, src_port);

#ifdef DEBUG  
  net_dump_ip(ip_get_src_ip(buf));
  uart_send(' ');
  uart_send_hex_byte_crlf(udp_get_src_port(ptr));
  net_dump_ip(ip_get_tgt_ip(buf));
  uart_send(' ');
  uart_send_hex_byte_crlf(udp_get_tgt_port(ptr));
#endif
  
  // enter UDP data
  off += UDP_DATA_OFF;
  ptr = buf + off;
  
  // swap bootp type
  if(ptr[BOOTP_OFF_OP] == BOOTP_REQUEST) {
    ptr[BOOTP_OFF_OP] = BOOTP_REPLY;
  } else {
    ptr[BOOTP_OFF_OP] = BOOTP_REQUEST;
  }

#ifdef DEBUG
  uart_send_hex_byte_crlf(ptr[BOOTP_OFF_OP]);
#endif
  
  return off;
}

void bootp_finish_swap_pkt(u08 *buf)
{
  ip_hdr_set_checksum(buf);
  udp_set_checksum(buf);
}

u08 bootp_begin_eth_pkt(u08 *buf, u08 op)
{
  eth_make_to_any(buf, ETH_TYPE_IPV4);
  return bootp_begin_pkt(buf + ETH_HDR_SIZE, op) + ETH_HDR_SIZE;
}

u16 bootp_finish_eth_pkt(u08 *buf, u16 size)
{
  return bootp_finish_pkt(buf + ETH_HDR_SIZE, size) + ETH_HDR_SIZE;
}

u08 bootp_begin_swap_eth_pkt(u08 *buf)
{
  eth_make_to_any(buf, ETH_TYPE_IPV4);

#ifdef DEBUG
  net_dump_mac(eth_get_src_mac(buf));
  uart_send_crlf();
  net_dump_mac(eth_get_tgt_mac(buf));
  uart_send_crlf();
#endif
  
  return bootp_begin_swap_pkt(buf + ETH_HDR_SIZE) + ETH_HDR_SIZE; 
}

void bootp_finish_swap_eth_pkt(u08 *buf)
{
  bootp_finish_swap_pkt(buf + ETH_HDR_SIZE);
}

u08 bootp_is_bootp_pkt(u08 *buf)
{
  if(udp_is_udp_pkt(buf)) {
    u08 *udp_buf = buf + ip_get_hdr_length(buf);
    u16 src_port = udp_get_src_port(udp_buf);
    u16 tgt_port = udp_get_tgt_port(udp_buf);
    return ((src_port == 67) && (tgt_port == 68)) ||
      ((src_port == 68) && (tgt_port == 67));
  } else {
    return 0;
  }
}

