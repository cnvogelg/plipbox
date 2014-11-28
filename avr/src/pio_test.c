/*
 * pio_test.c - test packet I/O
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

#include "pio_test.h"
#include "pio.h"
#include "param.h"
#include "uartutil.h"
#include "pkt_buf.h"
#include "net/net.h"
#include "timer.h"

void pio_test_begin(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[PIO_TEST] on\r\n"));

  pio_init(param.mac_addr, PIO_INIT_BROAD_CAST);
}

void pio_test_end(void)
{
  pio_exit();

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[PIO_TEST] off\r\n"));
}

void pio_test_worker(void)
{
  // incoming packet?
  if(pio_has_recv()) {
    u16 size;

    // measure packet receive
    timer_hw_reset();
    u08 result = pio_recv(pkt_buf, PKT_BUF_SIZE, &size);
    u16 delta = timer_hw_get();

    uart_send_time_stamp_spc();
    uart_send_pstring(PSTR("recv: "));
    if(result == PIO_OK) {
      // speed
      uart_send_pstring(PSTR("v="));
      u16 rate = timer_hw_calc_rate_kbs(size, delta);
      uart_send_rate_kbs(rate);

      // size
      uart_send_pstring(PSTR(" n="));
      uart_send_hex_word(size);
      uart_send_crlf();

      // send a reply: swap src/dst addr
      net_copy_mac(pkt_buf + 6, pkt_buf);
      net_copy_mac(param.mac_addr, pkt_buf + 6);

      timer_hw_reset();
      result = pio_send(pkt_buf, size);
      delta = timer_hw_get();

      uart_send_time_stamp_spc();
      uart_send_pstring(PSTR("send: "));
      if(result == PIO_OK) {
        // speed
        uart_send_pstring(PSTR("v="));
        u16 rate = timer_hw_calc_rate_kbs(size, delta);
        uart_send_rate_kbs(rate);

        // size
        uart_send_pstring(PSTR(" n="));
        uart_send_hex_word(size);
        uart_send_crlf();
      } else {
        uart_send_pstring(PSTR("ERROR="));
        uart_send_hex_byte(result);
        uart_send_crlf();
      }

    } else {
      uart_send_pstring(PSTR("ERROR="));
      uart_send_hex_byte(result);
      uart_send_crlf();
    }
  }
}
