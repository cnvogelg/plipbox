/*
 * net.h - tool functions for network protocols
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

#ifndef NET_H
#define NET_H

#include "global.h"

extern void net_init(const u08 mac[6], const u08 ip[4], const u08 gw[4], const u08 nm[4]);
 
extern const u08* net_get_mac(void);
extern const u08* net_get_ip(void);
extern const u08* net_get_gateway(void);
extern const u08* net_get_netmask(void);

extern void net_copy_my_mac(u08 *out);
extern void net_copy_my_ip(u08 *out);
extern void net_copy_zero_ip(u08 *out);

extern u08 net_compare_my_mac(const u08 *in);
extern u08 net_compare_my_ip(const u08 *in);
extern u08 net_compare_any_mac(const u08 *in);

extern void net_copy_mac(const u08 *in, u08 *out);
extern void net_copy_ip(const u08 *in, u08 *out);

extern void net_copy_any_mac(u08 *out);
extern void net_copy_zero_mac(u08 *out);

extern u16  net_get_word(const u08 *buf);
extern void net_put_word(u08 *buf, u16 value);

extern u32  net_get_long(const u08 *buf);
extern void net_put_long(u08 *buf, u32 value);

extern void net_dump_mac(const u08 *in);
extern void net_dump_ip(const u08 *in);

extern u08  net_compare_mac(const u08 *a, const u08 *b);
extern u08  net_compare_ip(const u08 *a, const u08 *b);

extern u08  net_is_my_subnet(const u08 *ip);

#endif
