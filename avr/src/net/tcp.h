/*
 * tcp.h - tool functions for TCP packets
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

#ifndef tcp_H
#define tcp_H

#include "global.h"
#include "net.h"
#include "ip.h"
   
   // byte offsets in TCP packets
#define TCP_SRC_PORT_OFF  0
#define TCP_TGT_PORT_OFF  2
#define TCP_SEQ_NUM_OFF   4
#define TCP_ACK_NUM_OFF   8
#define TCP_DATA_SIZE_OFF 12
#define TCP_FLAGS_OFF     12
#define TCP_WINDOW_OFF    14

  // flag masks
#define TCP_FLAGS_FIN     0x001
#define TCP_FLAGS_SYN     0x002
#define TCP_FLAGS_RST     0x004
#define TCP_FLAGS_PSH     0x008
#define TCP_FLAGS_ACK     0x010
#define TCP_FLAGS_URG     0x020
#define TCP_FLAGS_ECE     0x040
#define TCP_FLAGS_CWR     0x080
#define TCP_FLAGS_NS      0x100

inline const u08 *tcp_get_data_ptr(const u08 *tcp_buf) { return tcp_buf + (tcp_buf[TCP_DATA_SIZE_OFF] >> 4) * 4; }
inline u16  tcp_get_src_port(const u08 *tcp_buf) { return net_get_word(tcp_buf + TCP_SRC_PORT_OFF); }
inline u16  tcp_get_tgt_port(const u08 *tcp_buf) { return net_get_word(tcp_buf + TCP_TGT_PORT_OFF); }
inline u32  tcp_get_seq_num(const u08 *tcp_buf) { return net_get_long(tcp_buf + TCP_SEQ_NUM_OFF); }
inline u32  tcp_get_ack_num(const u08 *tcp_buf) { return net_get_long(tcp_buf + TCP_ACK_NUM_OFF); }
inline u16  tcp_get_flags(const u08 *tcp_buf) { return net_get_word(tcp_buf + TCP_FLAGS_OFF) & 0x1ff; }
inline u16  tcp_get_window_size(const u08 *tcp_buf) { return net_get_word(tcp_buf + TCP_WINDOW_OFF); }
  
#endif
