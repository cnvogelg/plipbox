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

plip_packet_t pkt;
u08 buf[0x5a];
u08 do_copy = 0;
u08 buf_pos = 0;

static u08 begin_rx_frame(plip_packet_t *pkt)
{
  if(pkt->size <= 0x5a) {
    do_copy = 1;
    buf_pos = 0;
  }
  return PLIP_STATUS_OK;
}

static u08 fill_rx_frame(u08 data)
{
  if(do_copy) {
    buf[buf_pos++] = data;
  }
  return PLIP_STATUS_OK;
}

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
  plip_init(begin_rx_frame, fill_rx_frame);
  
  uart_send_string("plip2slip");
  uart_send_crlf();
  
  while(1) {    
    // incoming packet?
    if(plip_is_recv_begin()) {
      led_on();
      
      // receive packet
      u08 status = plip_recv(&pkt);
      
      uart_send_hex_byte_crlf(status);
      uart_send_hex_word_crlf(pkt.size);
      uart_send_hex_word_crlf(buf_pos);
      uart_send_hex_word_crlf(pkt.crc);
      uart_send_hex_dword_crlf(pkt.type);
      for(u08 i=0;i<buf_pos;i++) {
        uart_send_hex_byte_spc(buf[i]);
        if((i & 15)==15) {
          uart_send_crlf();
        }
      }
      uart_send_crlf();
      
      led_off();
    }
  }
  
  return 0;
}
