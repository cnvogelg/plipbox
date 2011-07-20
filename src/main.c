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

#include <util/delay_basic.h>

#include "global.h"
#include "board.h"
#include "uart.h"
#include "timer.h"
#include "uartutil.h"
#include "par_low.h"

int main (void){
  // board init. e.g. switch off watchdog
  board_init();  
  // setup timer
  timer_init();
  // setup serial
  uart_init();
  // setup par
  par_low_init();
  
  uart_send_string("plip2slip");
  uart_send_crlf();
  
  u16 count = 0;
  u16 hash = 0;
  while(1) {
    led_on();
    
    // wait for strobe
    timer_100us = 0;
    while(!par_strobe_flag) {
      if(timer_100us == 5000) {
        // dump state
        uart_send_hex_word_crlf(count);
        count = 0;
        hash = 0;
      }
    }
    u08 d = par_strobe_data;  
    par_strobe_flag = 0;

    led_off();
    
    // get data
    count++;
    hash+=d;
    
    // do ack
    par_low_set_ack_lo();
    _delay_loop_1(6);
    par_low_set_ack_hi();
  }
  
  return 0;
}
