/*
 * icmp.h - work with ICMP packets
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

#ifndef ICMP_H
#define ICMP_H

#include "global.h"

#define ICMP_TYPE_OFF     0
#define ICMP_CODE_OFF     1
#define ICMP_CHECKSUM_OFF 2
#define ICMP_DATA_OFF     4
   
#define ICMP_PING_ID_OFF      4
#define ICMP_PING_SEQNUM_OFF  6

#define ICMP_TYPE_ECHO_REPLY    0
#define ICMP_TYPE_ECHO_REQUEST  8

// ----- ICMP Ping -----
extern u08 icmp_is_ping_request(const u08 *buf);
extern u08 icmp_is_ping_reply(const u08 *buf); 

extern u16 icmp_get_checksum(const u08 *buf);
extern u16 icmp_calc_checksum(const u08 *buf);
extern u08 icmp_validate_checksum(const u08 *buf);
extern void icmp_set_checksum(u08 *buf);

extern u16 icmp_get_ping_id(const u08 *buf);
extern u16 icmp_get_ping_seqnum(const u08 *buf);

extern void icmp_ping_request_to_reply(u08 *buf);
extern u08  icmp_begin_pkt(u08 *buf, const u08 *src_ip, const u08 *tgt_ip, u08 type, u08 code);
extern void icmp_finish_pkt(u08 *buf, u16 size);
extern u16  icmp_make_ping_request(u08 *buf, const u08 *src_ip, const u08 *tgt_ip, u16 id, u16 seq);

#endif
