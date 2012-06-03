/*
 * uartutil.c - serial utility routines
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plip2lip.
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

#include "uart.h"
#include "util.h"
#include "uartutil.h"

void uart_send_pstring(PGM_P data)
{
  while(1) {
    u08 c = pgm_read_byte_near(data);
    if(c == 0) {
      break;
    }
    uart_send(c);
    data++;
  }
}

void uart_send_string(const char *str)
{
  while(*str) {
    uart_send((u08)*str);
    str++;
  }
}

void uart_send_data(u08 *data,u08 len)
{
  for(u08 i=0;i<len;i++) {
    uart_send(data[i]);
  }
}

void uart_send_crlf(void)
{
  uart_send_pstring(PSTR("\r\n"));
}

void uart_send_spc(void)
{
  uart_send((u08)' ');
}

static u08 buf[8];

void uart_send_hex_byte_crlf(u08 data)
{
  byte_to_hex(data,buf);
  uart_send_data(buf,2);
  uart_send_crlf();
}

void uart_send_hex_byte_spc(u08 data)
{
  byte_to_hex(data,buf);
  uart_send_data(buf,2);
  uart_send_spc();
}

void uart_send_hex_word_crlf(u16 data)
{
  word_to_hex(data,buf);
  uart_send_data(buf,4);
  uart_send_crlf();
}

void uart_send_hex_word_spc(u16 data)
{
  word_to_hex(data,buf);
  uart_send_data(buf,4);
  uart_send_spc();
}

void uart_send_hex_dword_crlf(u32 data)
{
  dword_to_hex(data,buf);
  uart_send_data(buf,8);
  uart_send_crlf();
}
