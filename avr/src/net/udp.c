/*
 * udp.c - tool functions for UDP packets
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

#include "udp.h"
#include "ip.h"

u08 udp_begin_pkt(u08 *buf, const u08 *src_ip, u16 src_port, const u08 *tgt_ip, u16 tgt_port)
{
  u08 offset = ip_begin_pkt(buf, src_ip, tgt_ip, IP_PROTOCOL_UDP);
  
  u08 *udp_buf = buf + offset;
  net_put_word(udp_buf + UDP_SRC_PORT_OFF, src_port);
  net_put_word(udp_buf + UDP_TGT_PORT_OFF, tgt_port);
  
  return offset + UDP_DATA_OFF;
}

u16 udp_calc_checksum(const u08 *buf)
{
  const u08 *src_ip = ip_get_src_ip(buf);
  const u08 *tgt_ip = ip_get_tgt_ip(buf);
  u08 offset = ip_get_hdr_length(buf);
  u16 udp_size = udp_get_length(buf + offset);
  
  // calc UDP checksum
  u32 chkadd = 0;
  ip_add_checksum_ip(&chkadd, src_ip);
  ip_add_checksum_ip(&chkadd, tgt_ip);
  ip_add_checksum_word(&chkadd, IP_PROTOCOL_UDP);
  ip_add_checksum_word(&chkadd, udp_size);
  u16 num_words = udp_size >> 1;
  const u08 *udp_buf = buf + offset;
  for(u16 i=0;i<num_words;i++) {
    ip_add_checksum_ptr(&chkadd, udp_buf);
    udp_buf += 2;
  }
  return ip_finish_checksum(chkadd);
}

void udp_set_checksum(u08 *buf)
{
  u08 offset = ip_get_hdr_length(buf);
  u08 *udp_buf = buf + offset;
  udp_buf[UDP_CHECKSUM_OFF] = 0;
  udp_buf[UDP_CHECKSUM_OFF+1] = 0;
  u16 checksum = udp_calc_checksum(buf);
  net_put_word(udp_buf + UDP_CHECKSUM_OFF, ~checksum);
}

u16 udp_finish_pkt(u08 *buf, u16 data_size)
{
  u08 offset = ip_get_hdr_length(buf);

  // UDP size with header
  data_size += UDP_DATA_OFF;
  u08 *udp_buf = buf + offset;
  net_put_word(udp_buf + UDP_LENGTH_OFF, data_size);
  
  // calc UDP checksum
  net_put_word(udp_buf + UDP_CHECKSUM_OFF, 0);
  u16 chksum = ~ udp_calc_checksum(buf);
  net_put_word(udp_buf + UDP_CHECKSUM_OFF, chksum);
  
  // finish IP packet -> checksum
  ip_finish_pkt(buf, data_size + offset);
  return data_size + offset;
}
