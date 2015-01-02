/*
 * eth.h - working with ethernet packets
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

#ifndef ETH_H
#define ETH_H

#include "global.h"
#include "net.h"

#define ETH_OFF_TGT_MAC   0
#define ETH_OFF_SRC_MAC   6
#define ETH_OFF_TYPE      12
   
#define ETH_HDR_SIZE  14

#define ETH_TYPE_IPV4 0x800
#define ETH_TYPE_ARP  0x806   

// own magic eth types
#define ETH_TYPE_MAGIC_ONLINE	0xffff
#define ETH_TYPE_MAGIC_OFFLINE  0xfffe
#define ETH_TYPE_MAGIC_LOOPBACK 0xfffd

#define ETH_TYPE_MAGIC_LOOPBACK 0XFFFD

inline const u08* eth_get_tgt_mac(const u08 *pkt) { return pkt + ETH_OFF_TGT_MAC; }
inline const u08 *eth_get_src_mac(const u08 *pkt) { return pkt + ETH_OFF_SRC_MAC; }
inline u16 eth_get_pkt_type(const u08 *pkt) { return net_get_word(pkt + ETH_OFF_TYPE); }
inline u08 eth_is_arp_pkt(const u08 *pkt) { return eth_get_pkt_type(pkt) == ETH_TYPE_ARP; }
inline u08  eth_is_ipv4_pkt(const u08 *pkt) { return eth_get_pkt_type(pkt) == ETH_TYPE_IPV4; }  
inline void eth_set_pkt_type(u08 *pkt, u16 type) { net_put_word(pkt + ETH_OFF_TYPE, type); }

inline void eth_make_bcast(u08 *pkt, const u08 *my_mac) 
{
	net_copy_mac(net_bcast_mac, pkt + ETH_OFF_TGT_MAC);
	net_copy_mac(my_mac, pkt + ETH_OFF_SRC_MAC);
}

#endif
