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

u08 icmp_is_ping_request(const u08 *buf)
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
  return ip_hdr_calc_check_var(buf, off, num_words);  
}

u08 icmp_check(const u08 *buf)
{
  return icmp_sum_calc(buf) == 0xffff;
}

void icmp_calc_check(u08 *buf)
{
  // clear check
  buf[22] = buf[23] = 0;
  // calc check
  u16 check = ~ icmp_sum_calc(buf);
  // store check
  buf[22] = (u08)(check >> 8);
  buf[23] = (u08)(check & 0xff);  
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
  buf[20] = 0;
  icmp_calc_check(buf);
}

