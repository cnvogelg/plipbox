/*
 * uartutil.c - serial utility routines
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

#include "uart.h"
#include "util.h"
#include "uartutil.h"
#include "timer.h"

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

static u08 buf[12];

void uart_send_time_stamp_spc(void)
{
  u32 ts = time_stamp;
  dword_to_dec(ts, buf, 10, 4);
  buf[11] = ' ';
  uart_send_data(buf,12);
}

void uart_send_time_stamp_spc_ext(u32 ts)
{
  dword_to_dec(ts, buf, 10, 4);
  buf[11] = ' ';
  uart_send_data(buf,12);
}

void uart_send_rate_kbs(u16 kbs)
{
  dword_to_dec(kbs, buf, 6, 2);
  uart_send_data(buf,7);
  uart_send_pstring(PSTR(" KB/s"));
}

void uart_send_delta(u32 delta)
{
  // huge -> show upper hex
  if(delta > 0xffff) {
    buf[0] = '!';
    word_to_hex((u16)(delta >> 16), buf+1);
  }
  // for too large numbers use hex
  else if(delta > 9999) {
    buf[0] = '>';
    word_to_hex(delta, buf+1);
  } 
  // for smaller numbers use decimal
  else {
    buf[0] = '+';
    dword_to_dec(delta, buf+1, 4, 4);
  }
  uart_send_data(buf,5);
}

void uart_send_hex_byte(u08 data)
{
  byte_to_hex(data,buf);
  uart_send_data(buf,2);
}

void uart_send_hex_word(u16 data)
{
  word_to_hex(data,buf);
  uart_send_data(buf,4);
}

void uart_send_hex_dword(u32 data)
{
  dword_to_hex(data,buf);
  uart_send_data(buf,8);
}

#ifdef DEBUG
void uart_send_free_stack(void)
{
  u16 free = stack_free();
  uart_send_pstring(PSTR("free stack:"));
  uart_send_hex_word(free);
  uart_send_crlf();
}
#endif
