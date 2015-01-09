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
#include "pio_util.h"
#include "param.h"
#include "uartutil.h"
#include "main.h"
#include "stats.h"
#include "cmd.h"

u08 pio_test_loop(void)
{
  u08 reset = 0;

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[PIO_TEST] on\r\n"));

  pio_init(param.mac_addr, pio_util_get_init_flags());
  stats_reset();
  
  while(run_mode == RUN_MODE_PIO_TEST) {
    // handle commands
    reset = !cmd_worker();
    if(reset) {
      break;
    }

    // incoming packet?
    if(pio_has_recv()) {
      u16 size;
      if(pio_util_recv_packet(&size) == PIO_OK) {
        // handle ARP?
        if(!pio_util_handle_arp(size)) {
          // is it a UDP test packet?
          if(pio_util_handle_udp_test(size)) {
            // directly send back test packet
            pio_util_send_packet(size);
          }          
        }
      } else {
        stats_get(STATS_ID_PIO_RX)->err++;
      }
    }
  }

  stats_dump(0,1);
  pio_exit();

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[PIO_TEST] off\r\n"));

  return reset;
}
