/*
 * log.c - status log
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

#include "log.h"
#include "uart.h"
#include "uartutil.h"

#define LOG_SIZE  16

typedef struct {
  u32 time_stamp;
  u32 duration;
  u08 cmd;
  u08 result;
  u16 size;
} log_entry_t;

static log_entry_t entries[LOG_SIZE];
static u08 num;
static u08 pos;
static u16 total;

void log_reset(void)
{
  num = 0;
  pos = 0;
  total = 0;
}

void log_add(u32 time_stamp, u32 duration, u08 cmd, u08 result, u16 size)
{
  total++;
  log_entry_t *e;
  if(num == LOG_SIZE) {
    e = &entries[pos];
    pos ++;
    if(pos == LOG_SIZE) {
      pos = 0;
    }
  } else {
    e = &entries[pos];
    pos++;
    num++;
  }
  
  e->time_stamp = time_stamp;
  e->duration = duration;
  e->cmd = cmd;
  e->result = result;
  e->size = size;
}

void log_dump(void)
{
  // header
  uart_send_string("log: ts, delta, cmd, result, size  #");
  uart_send_hex_word(total);
  uart_send_crlf();
  
  int off = pos;
  for(int i=0;i<num;i++) {
    if(off == 0) {
      off = num - 1;
    } else {
      off--;
    }
    log_entry_t *e = &entries[off];

    // show entry
    uart_send_time_stamp_spc_ext(e->time_stamp);
    uart_send_time_stamp_spc_ext(e->duration);
    uart_send_hex_byte(e->cmd);    
    uart_send_spc();
    uart_send_hex_byte(e->result);
    uart_send_spc();
    uart_send_hex_word(e->size);
    uart_send_crlf();
  }
}
