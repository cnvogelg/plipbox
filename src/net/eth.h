/*
 * eth.h - working with ethernet packets
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

#ifndef ETH_H
#define ETH_H

#include "global.h"

#define ETH_HDR_SIZE  14

#define ETH_TYPE_IPV4 0x800
#define ETH_TYPE_ARP  0x806   

extern const u08* eth_get_tgt_mac(const u08 *pkt);
extern const u08* eth_get_src_mac(const u08 *pkt);
extern u16  eth_get_pkt_type(const u08 *pkt);
extern u08  eth_is_arp_pkt(const u08 *pkt);
extern u08  eth_is_ipv4_pkt(const u08 *pkt);
extern u08  eth_is_broadcast_tgt(const u08 *pkt);
extern u08  eth_is_tgt_me(const u08 *pkt);

extern void eth_make_reply(u08 *pkt);
extern void eth_make_to_tgt(u08 *pkt, u16 type, const u08 mac[6]);
extern void eth_make_to_any(u08 *pkt, u16 type);

extern void eth_dump(const u08 *pkt);

#endif
