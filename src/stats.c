/*
 * stats.c - manage device statistics
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

#include "stats.h"
#include "uartutil.h"
#include "uart.h"

stats_t stats;

void stats_reset(void)
{
  stats.pkt_rx_cnt = 0;
  stats.pkt_tx_cnt = 0;
  stats.pkt_rx_err = 0;
  stats.pkt_tx_err = 0;
  stats.pkt_rx_bytes = 0;
  stats.pkt_tx_bytes = 0;
}

void stats_dump(void)
{
  uart_send_string("rx_cnt: ");
  uart_send_hex_word_crlf(stats.pkt_rx_cnt);
  uart_send_string("rx_byte:");
  uart_send_hex_dword_crlf(stats.pkt_rx_bytes);

  u16 rx = stats.pkt_rx_err;
  if(rx > 0) {
    uart_send_string("rx_err: ");
    uart_send_hex_word_crlf(rx);
    uart_send_string(" last:");
    uart_send_hex_byte_crlf(stats.pkt_last_rx_err);
  }

  uart_send_string("tx_cnt: ");
  uart_send_hex_word_crlf(stats.pkt_tx_cnt);
  uart_send_string("tx_byte:");
  uart_send_hex_dword_crlf(stats.pkt_tx_bytes);

  u16 tx = stats.pkt_tx_err;
  if(tx > 0) {
    uart_send_string("tx_err:");
    uart_send_hex_word_crlf(tx);
    uart_send_string(" last:");
    uart_send_hex_byte_crlf(stats.pkt_last_tx_err);
  }
}
