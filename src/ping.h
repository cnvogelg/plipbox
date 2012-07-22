/*
 * ping.h - ICMP ping
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

#ifndef PING_H
#define PING_H

#include "global.h"

extern u08 ping_eth_send_request(const u08 *ip, u16 id, u16 seq);
extern void ping_eth_handle_packet(u08 *ip_buf, u16 ip_len);

extern u08 ping_plip_send_request(const u08 *ip, u16 id, u16 seq);
extern void ping_plip_handle_packet(u08 *ip_buf, u16 ip_len);

#endif
