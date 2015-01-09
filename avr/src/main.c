/*
 * main.c - main loop
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

#include <util/delay.h>

#include "global.h"
#include "board.h"
#include "uart.h"
#include "uartutil.h"
#include "timer.h"
#include "par_low.h"
#include "param.h"
#include "cmd.h"

#include "pb_test.h"
#include "pio_test.h"
#include "bridge_test.h"
#include "bridge.h"
#include "main.h"

u08 run_mode = RUN_MODE_BRIDGE;
u08 global_verbose = 0;

static void init_hw(void)
{
  // board init. e.g. switch off watchdog
  board_init();  
  // setup timer
  timer_init();
  // setup serial
  uart_init();
  // setup par
  par_low_init();
}

static void loop(void)
{
  init_hw();
  
  // send welcome
  uart_send_pstring(PSTR("\r\nWelcome to plipbox " VERSION " " BUILD_DATE "\r\n"));
  uart_send_pstring(PSTR("by lallafa (http://www.lallafa.de/blog)\r\n\r\n"));
  
  // param init
  param_init();  
  param_dump();
  uart_send_crlf();

  // help info
  uart_send_pstring(PSTR("Press <return> to enter command mode or <?> for key help\r\n"));

#ifdef DEBUG
  uart_send_free_stack();
#endif

  // select main loop depending on current run mode
  while(1) {
    u08 result = CMD_WORKER_IDLE;
    switch(run_mode) {
      case RUN_MODE_PB_TEST:
        result = pb_test_loop();
        break;
      case RUN_MODE_PIO_TEST:
        result = pio_test_loop();
        break;
      case RUN_MODE_BRIDGE_TEST:
        result = bridge_test_loop();
        break;
      case RUN_MODE_BRIDGE:
      default:
        result = bridge_loop();
        break;
    }
    // exit loop to perform reset
    if(result == CMD_WORKER_RESET) {
      break;
    }
  }

  // wait a bit
  uart_send_pstring(PSTR("resetting...\r\n"));
  _delay_ms(100);
}

int main(void)
{
  while(1) {
    loop();
  }
  return 0;
} 
