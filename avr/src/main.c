/*
 * main.c - main loop of plipbox
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

#include "global.h"
#include "board.h"
#include "uart.h"
#include "uartutil.h"
#include "timer.h"
#include "param.h"
#include "cmd.h"

#include "pio.h"
#include "pio_util.h"
#include "bridge.h"

#include "pkt_buf.h"

#include "proto_cmd.h"
#include "proto_cmd_shared.h"

// global switches
u08 global_verbose = 0;

int main(void)
{
  // board init. e.g. switch off watchdog
  board_init();
  // setup timer
  timer_init();
  // setup serial
  uart_init();
  
  // send welcome
  uart_send_pstring(PSTR("\r\nWelcome to plipbox " VERSION " " BUILD_DATE "\r\n"));
  uart_send_pstring(PSTR("by lallafa (http://www.lallafa.de/blog)\r\n\r\n"));
    // help info
  uart_send_pstring(PSTR("Press <return> to enter command mode or <?> for key help\r\n"));

  // param init
  param_init();  
  param_dump();
  uart_send_crlf();

#ifdef DEBUG
  uart_send_free_stack();
#endif

  // parallel proto init
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("proto: init\r\n"));
  proto_cmd_init();

  // packet i/o adapter init (eth)
  u08 ok = pio_init(pio_util_get_init_flags());
  bridge_init(ok == PIO_OK);

  // main loop
  while(1) {
    // handle parallel port commands and dispatch to proto_cmd_api_*
    u08 res = proto_cmd_handle();
    if(res == PROTO_CMD_HANDLE_RESET) {
      break;
    }

    // handle bridge/pio stuff
    bridge_handle();

    // handle commands
    res = cmd_worker();
    if(res & CMD_WORKER_RESET) {
      break;
    }
  }

  // wait a bit and reset
  uart_send_pstring(PSTR("resetting...\r\n"));
  board_reset();

  // never reach this
  return 0;
}
