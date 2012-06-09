/*
 * udp.h - tool functions for UDP packets
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

#ifndef UDP_H
#define UDP_H

#include "global.h"
#include "net.h"
   
   // byte offsets in UDP packets
#define UDP_SRC_PORT_OFF  0
#define UDP_TGT_PORT_OFF  2
#define UDP_LENGTH_OFF    4
#define UDP_CHECKSUM_OFF  6
#define UDP_DATA_OFF      8

extern u08 udp_begin_pkt(u08 *buf, const u08 *src_ip, u16 src_port, const u08 *tgt_ip, u16 tgt_port);
extern u16 udp_finish_pkt(u08 *buf, u16 data_size);
extern u16 udp_calc_checksum(const u08 *buf);

inline const u08 *udp_get_data_ptr(const u08 *udp_buf) { return udp_buf + UDP_DATA_OFF; }
inline u16  udp_get_src_port(const u08 *udp_buf) { return net_get_word(udp_buf + UDP_SRC_PORT_OFF); }
inline u16  udp_get_tgt_port(const u08 *udp_buf) { return net_get_word(udp_buf + UDP_TGT_PORT_OFF); }
inline u16  udp_get_length(const u08 *udp_buf) { return net_get_word(udp_buf + UDP_LENGTH_OFF); }
inline u16  udp_get_checksum(const u08 *udp_buf) { return net_get_word(udp_buf + UDP_CHECKSUM_OFF); }   
  
#endif
