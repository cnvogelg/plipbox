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

#include <avr/pgmspace.h>
#include "global.h"

// send a c string from PROGMEM
void uart_send_pstring(PGM_P data);
// send a c string
void uart_send_string(const char *data);
// send data
void uart_send_data(u08 *data,u08 size);
// send a CR+LF
void uart_send_crlf(void);
// send a Space
void uart_send_spc(void);

// send a hex byte
void uart_send_hex_byte_crlf(u08 data);
// send a hex byte
void uart_send_hex_byte_spc(u08 data);
// send a hex word
void uart_send_hex_word_crlf(u16 data);
// send a hex word
void uart_send_hex_word_spc(u16 data);
// send a hex6 dword
void uart_send_hex_dword_crlf(u32 data); 

#endif

