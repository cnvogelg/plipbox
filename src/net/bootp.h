/*
 * bootp.h - handle BOOTP messages
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

#ifndef BOOTP_H
#define BOOTP_H

#include "net.h"
   
#define BOOTP_REQUEST     1
#define BOOTP_REPLY       2
   
#define BOOTP_OFF_OP      0
#define BOOTP_OFF_HTYPE   1
#define BOOTP_OFF_HLEN    2
#define BOOTP_OFF_HOPS    3
#define BOOTP_OFF_XID     4
#define BOOTP_OFF_SECS    8
#define BOOTP_OFF_FLAGS   10
#define BOOTP_OFF_CIADDR  12
#define BOOTP_OFF_YIADDR  16
#define BOOTP_OFF_SIADDR  20
#define BOOTP_OFF_GIADDR  24
#define BOOTP_OFF_CHADDR  28
#define BOOTP_OFF_SNAME   44
#define BOOTP_OFF_FILE    108
#define BOOTP_OFF_VEND    236
   
#define BOOTP_MIN_SIZE    300
   
extern u08 bootp_begin_pkt(u08 *buf, u08 op);
extern u16 bootp_finish_pkt(u08 *buf);
extern u08 bootp_begin_swap_pkt(u08 *buf);
extern void bootp_finish_swap_pkt(u08 *buf);
   
extern u08 bootp_begin_eth_pkt(u08 *buf, u08 op);
extern u16 bootp_finish_eth_pkt(u08 *buf);
extern u08 bootp_begin_swap_eth_pkt(u08 *buf);
extern void bootp_finish_swap_eth_pkt(u08 *buf);

extern u08 bootp_is_bootp_pkt(u08 *buf);

inline u32 bootp_get_xid(const u08 *udp_buf) { return net_get_long(udp_buf + BOOTP_OFF_XID); }

#endif
