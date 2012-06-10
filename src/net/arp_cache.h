/*
 * arp_cache.h - manage the ARP cache
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

#ifndef ARP_CACHE_H
#define ARP_CACHE_H

#include "arp.h"
#include "param.h"
#include "net.h"

#define ARP_CACHE_SIZE (PARAM_NUM_ARP_IP + 1)

extern void arp_cache_init(void);
extern u08  arp_cache_handle_packet(u08 *ethbuf, u16 ethlen, net_tx_packet_func tx_func);
extern void arp_cache_worker(u08 *ethbuf, net_tx_packet_func tx_func);
extern void arp_cache_dump(void);
extern void arp_cache_clear(void);

/* return 0 if mac not known (yet) */
extern const u08 *arp_cache_find_mac(const u08 *ip);
inline const u08* arp_cache_get_gw_mac(void) { return arp_cache_find_mac(net_get_gateway()); }

#endif
