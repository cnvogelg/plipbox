/*
 * ip.h - work with IPv4 headers and ICMP packets
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

#ifndef IP_H
#define IP_H

#include "global.h"
#include "net.h"

/* list of IP protocol numbers */
#define IP_PROTOCOL_ICMP    0x01
#define IP_PROTOCOL_TCP     0x06
#define IP_PROTOCOL_UDP     0x11

#define IP_MIN_HDR_SIZE     20

#define IP_CHECKSUM_OFF     10
   
#define UDP_CHECKSUM_OFF    6
#define TCP_CHECKSUM_OFF    16

inline const u08 *ip_get_src_ip(const u08 *buf) { return buf + 12; }
inline const u08 *ip_get_tgt_ip(const u08 *buf) { return buf + 16; }
inline u16 ip_get_total_length(const u08 *buf) { return (u16)buf[2] << 8 | (u16)buf[3]; }
inline u08 ip_get_hdr_length(const u08 *buf) { return (buf[0] & 0xf) * 4; }
inline u08 ip_get_protocol(const u08 *buf) { return buf[9]; }

#endif