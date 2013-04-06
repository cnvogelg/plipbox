/*
 * bench.c - benchmark routines
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

#include "bench.h"
#include "timer.h"
#include "uart.h"
#include "uartutil.h"

typedef struct {
  u16 count;
  u16 bytes;
  u16 time;
} bench_t;
  
static bench_t current;
static bench_t stored;

void bench_begin(void)
{
  current.count = 0;
  current.bytes = 0;
  
  timer2_10ms = 0;
}

void bench_end(void)
{
  stored.time = timer2_10ms;
  stored.count = current.count;
  stored.bytes = current.bytes;
}

u16 bench_submit(u16 bytes)
{
  current.count ++;
  current.bytes += bytes;
  return current.count;
}

void bench_dump(void)
{
  uart_send_string("count:");
  uart_send_hex_word_crlf(stored.count);
  uart_send_string("bytes:");
  uart_send_hex_word_crlf(stored.bytes);
  uart_send_string("time: ");
  uart_send_hex_word_crlf(stored.time);
}