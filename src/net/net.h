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
#include "param.h"

/* typedefs for generic packet tx */
typedef void (*net_tx_packet_func)(const u08 *buf, u16);
 
inline const u08* net_get_mac(void) { return param.mac_addr; }
inline const u08* net_get_ip(void) { return param.ip_eth_addr; }
inline const u08* net_get_gateway(void) { return param.ip_gw_addr; }
inline const u08* net_get_netmask(void) { return param.ip_net_mask; }
inline const u08* net_get_p2p_me(void) { return param.ip_plip_addr; }
inline const u08* net_get_p2p_amiga(void) { return param.ip_amiga_addr; }

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

extern u08  net_is_my_subnet(const u08 *ip);

extern u08 net_parse_ip(const u08 *buf, u08 *ip);
extern u08 net_parse_mac(const u08 *buf, u08 *mac);

/* constants */
extern const u08 net_any_mac[6];
extern const u08 net_zero_mac[6];
extern const u08 net_zero_ip[4];
extern const u08 net_ones_ip[4];

/* convenience functions */
inline void net_copy_my_mac(u08 *out) { net_copy_mac(net_get_mac(), out); }
inline void net_copy_any_mac(u08 *out) { net_copy_mac(net_any_mac, out); }
inline void net_copy_zero_mac(u08 *out) { net_copy_mac(net_zero_mac, out); }

inline void net_copy_my_ip(u08 *out) { net_copy_ip(net_get_ip(), out); }
inline void net_copy_zero_ip(u08 *out) { net_copy_ip(net_zero_ip, out); }

inline u08 net_compare_my_mac(const u08 *in) { return net_compare_mac(net_get_mac(), in); }
inline u08 net_compare_any_mac(const u08 *in) { return net_compare_mac(net_any_mac, in); }

inline u08 net_compare_my_ip(const u08 *in) { return net_compare_ip(net_get_ip(), in); }

#endif
