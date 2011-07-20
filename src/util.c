/*
 * util.c - utilities
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

#include "util.h"

// convert to hex

u08 nybble_to_hex(u08 in)
{
  if(in<10)
    return '0' + in;
  else
    return 'A' + in - 10;
}

void byte_to_hex(u08 in,u08 *out)
{
  out[0] = nybble_to_hex(in >> 4);
  out[1] = nybble_to_hex(in & 0xf);
}

void word_to_hex(u16 in,u08 *out)
{
  byte_to_hex((u08)(in>>8),out);
  byte_to_hex((u08)(in&0xff),out+2);
}

void dword_to_hex(u32 addr,u08 *out)
{
  word_to_hex((u16)(addr>>16),out);
  word_to_hex((u16)(addr&0xffff),out+4);
}

// parse

u08 parse_nybble(u08 c,u08 *value)
{
  if((c>='a')&&(c<='f')) {
    *value = c + 10 - 'a';
    return 1;
  } 
  else if((c>='A')&&(c<='F')) {
    *value = c + 10 - 'A';
    return 1;
  } 
  else if((c>='0')&&(c<='9')) {
    *value = c - '0';
    return 1;
  } 
  else
    return 0;
}

u08 parse_byte(u08 *str,u08 *value)
{
  u08 val;
  if(!parse_nybble(str[0],&val))
    return 0;
  val <<= 4;
  if(!parse_nybble(str[1],value))
    return 0;
  *value |= val;
  return 1;
}

u08 parse_word(u08 *str,u16 *value)
{
  u08 val;
  if(!parse_byte(&str[0],&val))
    return 0;
  u08 val2;
  if(!parse_byte(&str[2],&val2))
    return 0;
  *value = (u16)val << 8 | val2;
  return 1;
}

u08 parse_dword(u08 *str,u32 *value)
{
  u08 val;
  if(!parse_byte(&str[0],&val))
    return 0;
  u08 val2;
  if(!parse_byte(&str[2],&val2))
    return 0;
  u08 val3;
  if(!parse_byte(&str[4],&val3))
    return 0;
  u08 val4;
  if(!parse_byte(&str[6],&val4))
    return 0;
  *value = (u32)val << 24 | (u32)val2 << 16 | (u32)val3 << 8 | val4;
  return 1;
}

