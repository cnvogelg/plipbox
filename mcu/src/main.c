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

#include "types.h"

#include "hw_system.h"
#include "hw_uart.h"
#include "hw_timer.h"
#include "hw_spi.h"

#include "uartutil.h"
#include "param.h"
#include "cmd.h"

#include "proto_cmd.h"
#include "proto_cmd_shared.h"
#include "mode.h"
#include "nic.h"

#define LOOP_DONE     0
#define LOOP_RESET    1
#define LOOP_RESTART  2

static int init_loop(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("waiting for driver...\r\n"));

  // main loop
  while(1) {
    // handle parallel port commands and dispatch to proto_cmd_api_*
    u08 res = proto_cmd_handle_init();
    if(res == PROTO_CMD_HANDLE_INIT) {
      return LOOP_DONE;
    }

    // handle commands
    res = cmd_worker();
    if(res == CMD_WORKER_RESET) {
      return LOOP_RESET;
    }
    else if(res == CMD_WORKER_EXIT) {
      return LOOP_DONE;
    }
  }
}

static int main_loop(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("entering main loop...\r\n"));

  mode_init();

  // main loop
  while(1) {
    // handle parallel port commands and dispatch to proto_cmd_api_*
    u08 res = proto_cmd_handle_main();
    if(res == PROTO_CMD_HANDLE_EXIT) {
      return LOOP_DONE;
    }
    if(res == PROTO_CMD_HANDLE_INIT) {
      return LOOP_RESTART;
    }

    // handle current mode
    mode_handle();

    // handle commands
    res = cmd_worker();
    if(res == CMD_WORKER_RESET) {
      return LOOP_RESET;
    }
    else if(res == CMD_WORKER_EXIT) {
      return LOOP_DONE;
    }
  }
}


int main(void)
{
  // board init. e.g. switch off watchdog
  hw_system_init();
  hw_timer_init();
  hw_uart_init();
  hw_spi_init();
  
  // send welcome
  uart_send_pstring(PSTR("\r\nWelcome to plipbox " VERSION " " BUILD_DATE "\r\n"));
  uart_send_pstring(PSTR("by lallafa (http://www.lallafa.de/blog)\r\n\r\n"));

  // param init
  u08 res = param_init();
  uart_send_pstring(PSTR("Device parameters "));
  if(res == HW_PERSIST_OK) {
    uart_send_pstring(PSTR("loaded.\r\n\r\n"));
  } else {
    uart_send_pstring(PSTR("RESET!\r\n\r\n"));
  }
  param_dump();
  uart_send_crlf();

#ifdef DEBUG
  uart_send_free_stack();
#endif

  // parallel proto init
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("proto: init\r\n"));
  proto_cmd_init();

  // main ops: waiting for driver and main loop
  int result = LOOP_DONE;
  while(1) {

    mode_init();
    nic_init();

    if(result == LOOP_DONE) {
      result = init_loop();
      if(result == LOOP_RESET) {
        break;
      }
    }

    result = main_loop();
    if(result == LOOP_RESET) {
      break;
    }
  }

  // wait a bit and reset
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("resetting...\r\n"));
  hw_system_reset();

  // never reach this
  return 0;
}
