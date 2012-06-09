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

#include <string.h>
#include <inttypes.h>

#include "ip.h"
#include "net.h"

/* ----- generic checksum ----- */

u16 ip_calc_checksum(const u08 *buf, u08 offset, int num_words)
{
  u32 sum = 0;
  u08 off = offset;
  for(u08 i=0;i<num_words;i++) {
    u16 add = net_get_word(buf+off);;
    off += 2;
    sum += add;
  }
  return (u16)(sum & 0xffff) + (u16)(sum >> 16);
}

void ip_adjust_checksum(u08 *buf, u08 offset, const u08 *old_ip, const u08 *new_ip)
{
  u16 sum = net_get_word(buf + offset);
  sum = ~sum;
  
  /* remove old ip */
  u08 off = 0;
  for(int i=0;i<2;i++) {
    u16 v = net_get_word(old_ip + off);
    u16 old_sum = sum;
    sum -= v;
    if(old_sum <= v) {
      sum --;
    }
    off += 2;
  }
  
  /* add new ip */
  off = 0;
  for(int i=0;i<2;i++) {
    u16 v = net_get_word(new_ip + off);
    u16 max = -(sum+1);
    sum += v;
    if(v >= max) {
      sum++; 
    }
    off += 2;
  }

  sum = ~sum;
  net_put_word(buf + offset, sum);
}

/* ----- IP header ----- */

u16 ip_hdr_calc_checksum(const u08 *buf)
{
  // header length in dword * 2 -> word
  u08 num_words = (buf[0] & 0xf) * 2;
  return ip_calc_checksum(buf, 0, num_words);  
}

void ip_hdr_set_checksum(u08 *buf)
{
  // clear check
  buf[10] = buf[11] = 0;
  // calc check 
  u16 check = ~ ip_hdr_calc_checksum(buf);
  // store check
  net_put_word(buf+10, check);
}

u08 ip_is_ipv4_protocol(const u08 *buf, u08 protocol)
{
  return
      ((buf[0] & 0xf0)==0x40) && // IPv4
      (buf[9]==protocol);
}

u08 ip_begin_pkt(u08 *buf, const u08 *src_ip, const u08 *tgt_ip, u08 protocol)
{
  memset(buf,0,20);  
  buf[0] = 0x45; // ipv4 + normal size of 5*4=20 bytes
  buf[6] = 0x40; // don't fragment
  buf[8] = 60; // time to live
  buf[9] = protocol;
  net_copy_ip(src_ip, buf+12);
  net_copy_ip(tgt_ip, buf+16);
  return 20; // offset
}

void ip_finish_pkt(u08 *buf, u16 size)
{
  // set total length
  net_put_word(buf+2,size);
  ip_hdr_set_checksum(buf);
}

