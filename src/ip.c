/*
 * ip.c - work with IPv4 headers and ICMP packets
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

static u16 sum_calc(const u08 *buf, u08 offset, int num_words)
{
  u32 sum = 0;
  u08 off = offset;
  for(u08 i=0;i<num_words;i++) {
    u16 add = (u16)buf[off] << 8 | (u16)buf[off+1];
    off += 2;
    sum += add;
  }
  return (u16)(sum & 0xffff) + (u16)(sum >> 16);
}

static u16 hdr_sum_calc(const u08 *buf)
{
  // header length in dword * 2 -> word
  u08 num_words = (buf[0] & 0xf) * 2;
  return sum_calc(buf, 0, num_words);  
}

u08 ip_hdr_check(const u08 *buf)
{ 
  return hdr_sum_calc(buf) == 0xffff;
}

void ip_hdr_calc_check(u08 *buf)
{
  // clear check
  buf[10] = buf[11] = 0;
  // calc check 
  u16 check = ~ hdr_sum_calc(buf);
  // store check
  buf[10] = (u08)(check >> 8);
  buf[11] = (u08)(check & 0xff);
}

u16 ip_hdr_get_size(const u08 *buf)
{
  return (u16)buf[2] << 8 | (u16)buf[3];
}

u08 ip_icmp_is_ping_request(const u08 *buf)
{
  return( 
      (buf[0]==0x45) && // IPv4
      (buf[9]==0x01) && // ICMP
      (buf[20]==0x08)  // echo request
      );
}

static u16 icmp_sum_calc(const u08 *buf)
{
  u08 num_hdr_words = (buf[0] & 0xf) * 2;
  u16 total_size = (u16)buf[2] << 8 | (u16)buf[3];
  u16 num_words = (total_size >> 1) - num_hdr_words;
  u08 off = num_hdr_words * 2;
  return sum_calc(buf, off, num_words);  
}

u08 ip_icmp_check(const u08 *buf)
{
  return icmp_sum_calc(buf) == 0xffff;
}

void ip_icmp_calc_check(u08 *buf)
{
  // clear check
  buf[22] = buf[23] = 0;
  // calc check
  u16 check = ~ icmp_sum_calc(buf);
  // store check
  buf[22] = (u08)(check >> 8);
  buf[23] = (u08)(check & 0xff);  
}

void ip_icmp_ping_request_to_reply(u08 *buf)
{
  // swap src and tgt ip
  for(u08 i=0;i<4;i++) {
    u08 src = buf[12+i];
    u08 tgt = buf[16+i];
    buf[12+i] = tgt;
    buf[16+i] = src;
  }
  
  // make an ICMP Ping Reply
  buf[20] = 0;
  ip_icmp_calc_check(buf);
}

