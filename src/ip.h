/*
 * ip.h - work with IPv4 headers and ICMP packets
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

#ifndef IP_H
#define IP_H

#include "global.h"

// ----- IP Header -----
extern u08 ip_hdr_check(const u08 *buf);
extern void ip_hdr_calc_check(u08 *buf);
extern u16 ip_hdr_get_size(const u08 *buf);

// ----- ICMP Ping -----
extern u08 ip_icmp_is_ping_request(const u08 *buf);
extern u08 ip_icmp_check(const u08 *buf);
extern void ip_icmp_calc_check(u08 *buf);
extern void ip_icmp_ping_request_to_reply(u08 *buf);

#endif