/*
 * net.c - tool functions for network protocols
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

#include "net.h"
#include "util.h"
#include "uartutil.h"

const u08 net_bcast_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
const u08 net_zero_mac[6] = { 0,0,0,0,0,0 };
const u08 net_zero_ip[4] = { 0,0,0,0 };
const u08 net_ones_ip[4] = { 255,255,255,255 };

void net_copy_mac(const u08 *in, u08 *out)
{
  for(int i=0;i<6;i++) {
    out[i] = in[i];
  }
}

void net_copy_ip(const u08 *in, u08 *out)
{
  for(int i=0;i<4;i++) {
    out[i] = in[i];
  }
}

u16  net_get_word(const u08 *buf)
{
  return (u16)buf[0] << 8 | (u16)buf[1];
}

void net_put_word(u08 *buf, u16 value)
{
  buf[0] = (u08)(value >> 8);
  buf[1] = (u08)(value & 0xff);
}

u32  net_get_long(const u08 *buf)
{
  return (u32)buf[0] << 24 | (u32)buf[1] << 16 | (u32)buf[2] << 8 | (u32)buf[3];
}

void net_put_long(u08 *buf, u32 value)
{
  buf[0] = (u08)(value >> 24);
  buf[1] = (u08)((value >> 16) & 0xff);
  buf[2] = (u08)((value >> 8) & 0xff);
  buf[3] = (u08)(value & 0xff);
}

static char *mac_str = "00:00:00:00:00:00";
static char *ip_str = "000.000.000.000";

void net_dump_mac(const u08 *in)
{
  int pos = 0;
  for(int i=0;i<6;i++) {
    byte_to_hex(in[i],(u08 *)(mac_str+pos));
    pos += 3;
  }
  uart_send_string(mac_str);
}

u08 net_parse_ip(const u08 *buf, u08 *ip)
{
  for(int i=0;i<4;i++) {
    u08 value;
    u08 digits = parse_byte_dec(buf,&value);
    if(digits == 0) {
      return 0;
    }
    buf += digits + 1;
    ip[i] = value;
  }
  return 1;
}

u08 net_parse_mac(const u08 *buf, u08 *mac)
{
  for(int i=0;i<6;i++) {
    u08 value;
    if(!parse_byte(buf, &value)) {
      return 0;
    }
    buf += 3;
    mac[i] = value;
  }
  return 1;
}

void net_dump_ip(const u08 *in)
{
  int pos = 0;
  for(int i=0;i<4;i++) {
    byte_to_dec(in[i],(u08 *)(ip_str+pos));
    pos += 4;
  }
  uart_send_string(ip_str);
}

u08  net_compare_mac(const u08 *a, const u08 *b)
{
  for(int i=0;i<6;i++) {
    if(a[i] != b[i]) {
      return 0;
    }
  }
  return 1;
}

u08  net_compare_ip(const u08 *a, const u08 *b)
{
  for(int i=0;i<4;i++) {
    if(a[i] != b[i]) {
      return 0;
    }
  }
  return 1;
}
