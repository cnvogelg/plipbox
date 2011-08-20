/*
 * main.c - main loop
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

#include "global.h"
#include "board.h"
#include "uart.h"
#include "timer.h"
#include "par_low.h"
#include "param.h"
#include "stats.h"
#include "ser_parse.h"
#include "cmd.h"

#include "transfer.h"
#include "ping_plip.h"
#include "ping_slip.h"

int main (void){
  // board init. e.g. switch off watchdog, init led
  board_init();  
  // setup timer
  timer_init();
  // setup serial
  uart_init();
  // setup par
  par_low_init();
  // param init
  param_init();
  
  // setup command handler
  ser_parse_set_cmd_func(cmd_parse);
  ser_parse_set_cmd_enter_leave_func(stats_capture_timing, stats_reset_timing);
  
  // enable uart
  uart_start_reception();

  // main loop
  while(1) {
    switch(param.mode) {
      case PARAM_MODE_TRANSFER:
        transfer_loop();
        break;
      case PARAM_MODE_PING_PLIP:
        ping_plip_loop();
        break;
      case PARAM_MODE_PING_SLIP:
        ping_slip_loop();
        break;
    }
  }

  return 0;
} 
