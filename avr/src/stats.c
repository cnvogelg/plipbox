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
#include "uart.h"

stats_t stats;

void stats_reset(void)
{
  stats.rx_cnt = 0;
  stats.tx_cnt = 0;
  stats.rx_err = 0;
  stats.tx_err = 0;
  stats.rx_bytes = 0;
  stats.tx_bytes = 0;
  stats.rx_drop = 0;
  stats.tx_drop = 0;
  stats.rx_filter = 0;
  stats.tx_filter = 0;
  stats.last_rx_err = 0;
  stats.last_tx_err = 0;
}

void stats_dump(void)
{
  // ----- rx -----
  uart_send_pstring(PSTR("rx_cnt: "));
  uart_send_hex_word(stats.rx_cnt);
  uart_send_crlf();
  uart_send_pstring(PSTR("rx_byte:"));
  uart_send_hex_dword(stats.rx_bytes);
  uart_send_crlf();

  u16 rx = stats.rx_err;
  if(rx > 0) {
    uart_send_pstring(PSTR("rx_err: "));
    uart_send_hex_word(rx);
    uart_send_crlf();
    uart_send_pstring(PSTR(" last:"));
    uart_send_hex_byte(stats.last_rx_err);
    uart_send_crlf();
  }

  rx = stats.rx_drop;
  if(rx > 0) {
    uart_send_pstring(PSTR("rx_drop:"));
    uart_send_hex_word(rx);
    uart_send_crlf();
  }

  rx = stats.rx_filter;
  if(rx > 0) {
    uart_send_pstring(PSTR("rx_filt:"));
    uart_send_hex_word(rx);
    uart_send_crlf();
  }

  // ----- tx -----
  uart_send_pstring(PSTR("tx_cnt: "));
  uart_send_hex_word(stats.tx_cnt);
  uart_send_crlf();
  uart_send_pstring(PSTR("tx_byte:"));
  uart_send_hex_dword(stats.tx_bytes);
  uart_send_crlf();

  u16 tx = stats.tx_err;
  if(tx > 0) {
    uart_send_pstring(PSTR("tx_err: "));
    uart_send_hex_word(tx);
    uart_send_crlf();
    uart_send_pstring(PSTR(" last:"));
    uart_send_hex_byte(stats.last_tx_err);
    uart_send_crlf();
  }
  
  tx = stats.tx_drop;
  if(tx > 0) {
    uart_send_pstring(PSTR("tx_drop:"));
    uart_send_hex_word(tx);
    uart_send_crlf();
  }

  tx = stats.tx_filter;
  if(tx > 0) {
    uart_send_pstring(PSTR("tx_filt:"));
    uart_send_hex_word(tx);
    uart_send_crlf();
  }
}
