/*
 * net.h - tool functions for network protocols
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

#ifndef NET_H
#define NET_H

#include "global.h"

/* typedefs for generic packet tx */
typedef void (*net_tx_packet_func)(const u08 *buf, u16);
 
extern void net_copy_mac(const u08 *in, u08 *out);
extern void net_copy_ip(const u08 *in, u08 *out);

extern u08  net_compare_mac(const u08 *a, const u08 *b);
extern u08  net_compare_ip(const u08 *a, const u08 *b);

extern u16  net_get_word(const u08 *buf);
extern void net_put_word(u08 *buf, u16 value);

extern u32  net_get_long(const u08 *buf);
extern void net_put_long(u08 *buf, u32 value);

extern void net_dump_mac(const u08 *in);
extern void net_dump_ip(const u08 *in);

extern u08 net_parse_ip(const u08 *buf, u08 *ip);
extern u08 net_parse_mac(const u08 *buf, u08 *mac);

/* constants */
extern const u08 net_bcast_mac[6];
extern const u08 net_zero_mac[6];
extern const u08 net_zero_ip[4];
extern const u08 net_ones_ip[4];

/* convenience functions */
static inline void net_copy_bcast_mac(u08 *out) { net_copy_mac(net_bcast_mac, out); }
static inline void net_copy_zero_mac(u08 *out) { net_copy_mac(net_zero_mac, out); }

static inline void net_copy_zero_ip(u08 *out) { net_copy_ip(net_zero_ip, out); }
static inline u08 net_compare_bcast_ip(const u08 *in) { return net_compare_ip(net_ones_ip, in); }

static inline u08 net_compare_bcast_mac(const u08 *in) { return net_compare_mac(net_bcast_mac, in); }
static inline u08 net_compare_zero_mac(const u08 *in) { return net_compare_mac(net_zero_mac, in); }

#endif
