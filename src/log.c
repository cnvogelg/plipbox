/*
 * log.c - status log
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

#include "log.h"
#include "uart.h"
#include "uartutil.h"

#define LOG_SIZE  16

static u08 codes[LOG_SIZE];
static u08 num;
static u08 pos;
static u16 total;

void log_init(void)
{
  num = 0;
  pos = 0;
  total = 0;
}

void log_add(u08 code)
{
  total++;
  if(num == LOG_SIZE) {
    codes[pos] = code;
    pos ++;
    if(pos == LOG_SIZE) {
      pos = 0;
    }
  } else {
    codes[pos] = code;
    pos++;
    num++;
  }
}

void log_dump(void)
{
  uart_send_string("log: ");
  uart_send_hex_word_crlf(total);
  int off = pos;
  for(int i=0;i<num;i++) {
    uart_send_hex_byte_crlf(codes[off]);
    if(off == 0) {
      off = LOG_SIZE -1 ;
    } else {
      off--;
    }
  }
}
