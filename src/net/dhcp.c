/*
 * dhcp.c - handle DHCP messages
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

#include "dhcp.h"
#include "bootp.h"
#include "eth.h"

u16 dhcp_begin_pkt(u08 *buf, u08 op)
{
  u08 offset = bootp_begin_pkt(buf, op);
  u08 *ptr = buf + offset + BOOTP_OFF_VEND;
  
  // DHCP magic
  ptr[0] = 0x63;
  ptr[1] = 0x82;
  ptr[2] = 0x53;
  ptr[3] = 0x63;
  
  return offset + BOOTP_OFF_VEND + 4;
}

u16 dhcp_finish_pkt(u08 *buf)
{
  return bootp_finish_pkt(buf);
}

u16 dhcp_begin_eth_pkt(u08 *buf, u08 op)
{
  eth_make_to_any(buf, ETH_TYPE_IPV4);
  return dhcp_begin_pkt(buf + ETH_HDR_SIZE, op) + ETH_HDR_SIZE;
}

u16 dhcp_finish_eth_pkt(u08 *buf)
{
  return dhcp_finish_pkt(buf + ETH_HDR_SIZE) + ETH_HDR_SIZE;
}

u08 *dhcp_add_type(u08 *buf, u08 type)
{
  buf[0] = 53;
  buf[1] = 1;
  buf[2] = type;
  return buf+3;
}
  
u08 *dhcp_add_end(u08 *buf)
{
  buf[0] = 0xff;
  return buf+1;
}
