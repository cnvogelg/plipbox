/*
 * uartutil.h - serial utility routines
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

#ifndef UARTUTIL_H
#define UARTUTIL_H

#include "global.h"

// send a c string
u08 uart_send_string(const char *data);
// send data
u08 uart_send_data(u08 *data,u08 size);
// send a CR+LF
u08 uart_send_crlf(void);

// send a hex byte
u08 uart_send_hex_byte_crlf(u08 data);
// send a hex word
u08 uart_send_hex_word_crlf(u16 data);
// send a hex6 dword
u08 uart_send_hex_dword6_crlf(u32 data); 

#endif

