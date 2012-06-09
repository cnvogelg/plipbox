/*
 * arp.h - handle ARP protocol
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

#ifndef ARP_H
#define ARP_H

#include "global.h"

extern u08 arp_is_ipv4(const u08 *buf, u16 len);
extern u08 arp_is_req_for_me(const u08 *buf);

extern void arp_make_reply(u08 *buf);
extern void arp_send_request(u08 *buf, const u08 *ip);

extern void arp_dump(const u08 *buf);

/* return 0 if not handled, >0 if was handled 
note: pass the raw ethernet frame!
*/
extern u08 arp_handle_packet(u08 *ethbuf, u16 ethlen);

/* copy gw mac or return 0 if gw not known */
extern const u08* arp_get_gw_mac(void);
extern const u08 *arp_find_mac(u08 *buf, const u08 *ip);

/* arp cache */
#define ARP_CACHE_INVALID   0xff

extern void arp_cache_init(void);
extern u08 *arp_cache_get_mac(u08 index);
extern u08  arp_cache_find_ip(const u08 *ip);
extern u08  arp_cache_add(const u08 *ip, const u08 *mac);
extern void arp_cache_update(u08 index, const u08 *mac);

#endif
