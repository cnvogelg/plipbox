/*
 * only_plip_rx.c - test mode: only plip rx, slip tx
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

#include "only_plip_rx.h"
#include "plip_rx.h"
#include "param.h"
#include "ser_parse.h"
#include "board.h"
#include "bench.h"
#include "pkt_buf.h"
#include "stats.h"

void only_plip_rx_loop(void)
{
  u16 count = 0;
  u16 error = 0;
  
  plip_rx_init();
  ser_parse_set_data_func(0); // use serial echo

  bench_begin();
  
  // ----- main loop -----
  led_green_on(); 
  while(param.mode == PARAM_MODE_ONLY_PLIP_RX) {
    // serial handling
    ser_parse_worker();
    
    // receive PLIP
    u08 status = plip_rx_worker();    
    if(status != PLIP_STATUS_IDLE) {
      
      if(status == PLIP_STATUS_OK) {
        bench_submit(pkt.size);
      } else {
        error ++;
      }
      
      // give summary
      count ++;
      if(count == 256) {
        count = 0;
        if(error>0) {
          led_red_on();
          error = 0;
        } else {
          led_red_off();
        }        
        bench_end();
        bench_begin();
      }
    }
  }
  led_green_off();
  
  bench_end();
}
