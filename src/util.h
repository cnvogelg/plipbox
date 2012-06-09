/*
 * util.h - utilities
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

#ifndef UTIL_H
#define UTIL_H

#include "global.h"

// ----- conversion functions -----
// convert nybble to hex char
extern u08 nybble_to_hex(u08 in);
// convert u08 to 2 hex chars
extern void byte_to_hex(u08 in,u08 *out);
// convert word to 4 hex chars
extern void word_to_hex(u16 in,u08 *out);
// convert dword to 6 hex chars
extern void dword_to_hex(u32 in,u08 *out);

// ----- parse functions: 01=ok, 00=error -----
// parse a nybble
extern u08 parse_nybble(u08 in,u08 *value);
// parse a byte
extern u08 parse_byte(const u08 *str,u08 *value);
// parse a word
extern u08 parse_word(const u08 *str,u16 *value);
// parse a 6 byte dword
extern u08 parse_dword(const u08 *str,u32 *value);

#endif

