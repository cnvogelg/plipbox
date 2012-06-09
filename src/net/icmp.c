/*
 * icmp.c - work with ICMP packets
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

#include "ip.h"
#include "icmp.h"
#include "net.h"

u08 icmp_is_ping_request(const u08 *buf)
{
  return buf[ip_get_hdr_length(buf) + ICMP_TYPE_OFF] == ICMP_TYPE_ECHO_REQUEST;
}

u08 icmp_is_ping_reply(const u08 *buf)
{
  return buf[ip_get_hdr_length(buf) + ICMP_TYPE_OFF] == ICMP_TYPE_ECHO_REPLY;  
}

u08 icmp_begin_pkt(u08 *buf, const u08 *src_ip, const u08 *tgt_ip, u08 type, u08 code)
{
  u08 off = ip_begin_pkt(buf, src_ip, tgt_ip, IP_PROTOCOL_ICMP);
  buf[off + ICMP_TYPE_OFF] = type;
  buf[off + ICMP_CODE_OFF] = code;
  return off + ICMP_DATA_OFF;
}

void icmp_finish_pkt(u08 *buf, u16 size)
{
  ip_finish_pkt(buf, size);
  icmp_set_checksum(buf);
}

u16 icmp_make_ping_request(u08 *buf, const u08 *src_ip, const u08 *tgt_ip, u16 id, u16 seq)
{
  u08 data_off = icmp_begin_pkt(buf, src_ip, tgt_ip, ICMP_TYPE_ECHO_REQUEST, 0);

  // quench
  net_put_word(buf+data_off, id);
  net_put_word(buf+data_off+2, seq);
  
  icmp_finish_pkt(buf, data_off +4);
  return data_off + 4;
}

u16 icmp_get_checksum(const u08 *buf)
{
  u08 off = ip_get_hdr_length(buf) + ICMP_CHECKSUM_OFF;
  return net_get_word(buf + off);
}

u16 icmp_calc_checksum(const u08 *buf)
{
  u08 hdr_len = ip_get_hdr_length(buf);
  u16 total_len = ip_get_total_length(buf);
  u16 num_words = (total_len - hdr_len) >> 1; // bytes -> words
  return ip_calc_checksum(buf, hdr_len, num_words);  
}

u08 icmp_validate_checksum(const u08 *buf)
{
  return icmp_calc_checksum(buf) == 0xffff;
}

void icmp_set_checksum(u08 *buf)
{
  u08 off = ip_get_hdr_length(buf) + ICMP_CHECKSUM_OFF;
  
  // clear check
  buf[off] = buf[off+1] = 0;
  // calc check
  u16 check = ~ icmp_calc_checksum(buf);
  // store check
  net_put_word(buf + off, check);
}

void icmp_ping_request_to_reply(u08 *buf)
{
  // swap src and tgt ip
  for(u08 i=0;i<4;i++) {
    u08 src = buf[12+i];
    u08 tgt = buf[16+i];
    buf[12+i] = tgt;
    buf[16+i] = src;
  }
  
  // make an ICMP Ping Reply
  buf[ip_get_hdr_length(buf) + ICMP_TYPE_OFF] = 0;
  icmp_set_checksum(buf);
}

u16 icmp_get_ping_id(const u08 *buf)
{
  return net_get_word(buf + ip_get_hdr_length(buf) + ICMP_PING_ID_OFF);
}

u16 icmp_get_ping_seqnum(const u08 *buf)
{
  return net_get_word(buf + ip_get_hdr_length(buf) + ICMP_PING_SEQNUM_OFF);  
}

