/*
 * stats.c - manage device statistics
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

#include "stats.h"
#include "uartutil.h"
#include "hw_uart.h"

stats_t stats[STATS_ID_NUM];

void stats_reset(void)
{
  for(u08 i=0;i<STATS_ID_NUM;i++) {
    stats_t *s = &stats[i];
    s->bytes = 0;
    s->cnt = 0;
    s->err = 0;
    s->drop = 0;
    s->max_rate = 0;
  }
}

void stats_update_ok(u08 id, u16 size, u16 rate)
{
  stats_t *s = &stats[id];
  s->cnt++;
  s->bytes += size;
  if(rate > s->max_rate) {
    s->max_rate = rate;
  }
}

static void dump_line(u08 id)
{
  const stats_t *s = &stats[id];

  uart_send_hex_word(s->cnt);
  uart_send_spc();
  uart_send_hex_dword(s->bytes);
  uart_send_spc();
  uart_send_hex_word(s->err);
  uart_send_spc();
  uart_send_hex_word(s->drop);
  uart_send_spc();
  uart_send_rate_kbs(s->max_rate);
  uart_send_spc();

  PGM_P str;
  switch(id) {
    case STATS_ID_PB_RX:
      str = PSTR("rx plipbox");
      break;
    case STATS_ID_PIO_RX:
      str = PSTR("rx pio");
      break;
    case STATS_ID_PB_TX:
    case STATS_ID_PIO_TX:
      str = PSTR("tx");
      break;
    default:
      str = PSTR("?");
      break;
  }
  uart_send_pstring(str);

  uart_send_crlf();
}

static void dump_header(void)
{
  uart_send_pstring(PSTR("cnt  bytes    err  drop rate\r\n"));  
}

void stats_dump_all(void)
{
  dump_header();
  for(u08 i=0;i<STATS_ID_NUM;i++) {
    dump_line(i);
  }
}

void stats_dump(u08 pb, u08 pio)
{
  dump_header();
  if(pb) {
    dump_line(STATS_ID_PB_RX);
    dump_line(STATS_ID_PB_TX);
  }
  if(pio) {
    dump_line(STATS_ID_PIO_RX);
    dump_line(STATS_ID_PIO_TX);
  }
}
