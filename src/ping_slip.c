/*
 * ping_plip.c - ping slip side test mode
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

#include "ping_slip.h"
#include "slip.h"
#include "bench.h"
#include "board.h"
#include "pkt_buf.h"
#include "uart.h"
#include "ser_parse.h"
#include "ip.h"
#include "param.h"

static u16 pos;
static u16 size;

static void slip_data(u08 data)
{
  if(pos < PKT_BUF_SIZE) {
    pkt_buf[pos] = data;
    pos++;
  }
}

static void slip_end(void)
{
  size = pos;
  pos = 0;
}

void ping_slip_loop(void)
{
  slip_push_init(slip_data, slip_end);
  ser_parse_set_data_func(slip_push);
  bench_begin();

  pos = 0;
  u16 count = 0;

  led_green_on(); 
  while(param.mode == PARAM_MODE_PING_SLIP) {
    // receive from serial and trigger slip_push
    ser_parse_worker();
    
    // do we receive a slip end?
    if(size > 0) {
      // stop receiption
      uart_stop_reception();
      led_green_off();
      
      if(ip_icmp_is_ping_request(pkt_buf)) {
        // make reply
        ip_icmp_ping_request_to_reply(pkt_buf);

        // send reply
        slip_send_end();
        for(u16 i=0;i<size;i++) {
          slip_send(pkt_buf[i]);
        }
        slip_send_end();
        
        // do bench marks
        count++;
        if(count == 256) {
          count = 0;
          bench_end();
          bench_begin();
        }
      }
      
      // start receiption again
      led_green_on();
      uart_start_reception();
      size = 0;
    }
  }
  led_green_off();
  
  bench_end();
}

