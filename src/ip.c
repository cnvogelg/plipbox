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

/* ----- IP header ----- */

u16 ip_hdr_get_checksum(const u08 *buf)
{
  // header length in dword * 2 -> word
  u08 num_words = (buf[0] & 0xf) * 2;
  return ip_calc_checksum(buf, 0, num_words);  
}

u08 ip_hdr_validate_checksum(const u08 *buf)
{ 
  return ip_hdr_get_checksum(buf) == 0xffff;
}

void ip_hdr_set_checksum(u08 *buf)
{
  // clear check
  buf[10] = buf[11] = 0;
  // calc check 
  u16 check = ~ ip_hdr_get_checksum(buf);
  // store check
  net_put_word(buf+10, check);
}

u16 ip_get_total_length(const u08 *buf)
{
  return (u16)buf[2] << 8 | (u16)buf[3];
}

u08 ip_get_hdr_length(const u08 *buf)
{
  return (buf[0] & 0xf) * 4;
}

u08 ip_is_ipv4_protocol(const u08 *buf, u08 protocol)
{
  return
      ((buf[0] & 0xf0)==0x40) && // IPv4
      (buf[9]==protocol);
}

const u08 *ip_get_src_ip(const u08 *buf)
{
  return buf + 12;
}

const u08 *ip_get_tgt_ip(const u08 *buf)
{
  return buf + 16;
}

void ip_set_src_ip(u08 *buf, const u08 *ip)
{
  net_copy_ip(ip, buf + 12);
}

void ip_set_tgt_ip(u08 *buf, const u08 *ip)
{
  net_copy_ip(ip, buf + 16);
}

u08 ip_begin_pkt(u08 *buf, const u08 *tgt_ip, u08 protocol)
{
  memset(buf,0,20);  
  buf[0] = 0x45; // ipv4 + normal size of 5*4=20 bytes
  buf[6] = 0x40; // don't fragment
  buf[8] = 60; // time to live
  buf[9] = protocol;
  net_copy_my_ip(buf+12);
  net_copy_ip(tgt_ip, buf+16);
  return 20; // offset
}

void ip_finish_pkt(u08 *buf, u16 size)
{
  // set total length
  net_put_word(buf+2,size);
  ip_hdr_set_checksum(buf);
}

