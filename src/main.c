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
#include "plip.h"
#include "ip.h"
#include "slip.h"
#include "ser_parse.h"

static u08 begin_end_rx_to_slip(plip_packet_t *pkt)
{
  if(slip_send_end()) {
    return PLIP_STATUS_OK;
  } else {
    return PLIP_STATUS_CALLBACK_FAILED;
  }
}

static u08 fill_rx_to_slip(u08 *data)
{
  u08 d = *data;
  if(slip_send(d)) {
    return PLIP_STATUS_OK;
  } else {
    return PLIP_STATUS_CALLBACK_FAILED;
  }
}

static plip_packet_t pkt;

int main (void){
  // board init. e.g. switch off watchdog
  board_init();  
  // setup timer
  timer_init();
  // setup serial
  uart_init();
  // setup par
  par_low_init();
  
  // setup plip functions
  plip_recv_init(begin_end_rx_to_slip, fill_rx_to_slip, begin_end_rx_to_slip);

  // enable uart
  uart_start_reception();

  // light green READY and enter main loop
  led_green_on();
  u16 error_count = 0;
  while(1) {
    // incoming packet?
    if(plip_is_recv_begin()) {
      led_green_off();
      uart_stop_reception();
      
      // receive packet -> transfer to slip
      u08 status = plip_recv(&pkt);
      if(status != PLIP_STATUS_OK) {
        error_count = 0x2fff;
      }

      uart_start_reception();
      led_green_on();
    }
    
    // update error LED
    if(error_count > 1) {
      led_red_on();
      error_count--;
    } else if(error_count == 1) {
      led_red_off();
    }
    
    // trigger serial state parser
    ser_parse_worker();
  }
  return 0;
}
