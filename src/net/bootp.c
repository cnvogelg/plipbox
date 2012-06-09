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
   
u16 bootp_finish_pkt(u08 *buf)
{
  return udp_finish_pkt(buf, BOOTP_MIN_SIZE);
}

u08 bootp_begin_eth_pkt(u08 *buf, u08 op)
{
  eth_make_to_any(buf, ETH_TYPE_IPV4);
  return bootp_begin_pkt(buf + ETH_HDR_SIZE, op) + ETH_HDR_SIZE;
}

u16 bootp_finish_eth_pkt(u08 *buf)
{
  return bootp_finish_pkt(buf + ETH_HDR_SIZE) + ETH_HDR_SIZE;
}
