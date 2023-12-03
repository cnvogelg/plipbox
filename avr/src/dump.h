/*
 * dump.h - helper functions for debugging
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

#ifndef DUMP_H
#define DUMP_H

#include "types.h"

extern void dump_eth_pkt(const u08 *eth_buf, u16 size);
extern void dump_arp_pkt(const u08 *arp_buf);
extern void dump_ip_pkt(const u08 *ip_buf);
extern void dump_ip_protocol(const u08 *ip_buf);

extern void dump_line(const u08 *eth_buf, u16 size);

#endif
