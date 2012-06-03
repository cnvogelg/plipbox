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

#include <avr/delay.h>

#include "global.h"
#include "board.h"
#include "uart.h"
#include "timer.h"
#include "par_low.h"
#include "param.h"
#include "stats.h"
#include "spi.h"
#include "enc28j60.h"
#include "net.h"
#include "uartutil.h"
	 
#include "eth_rx.h"
#include "eth_tx.h"
#include "eth_state.h"
#include "plip_state.h"

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
  // spi init
  spi_init();
}

static void init_eth(void)
{  
  uart_send_pstring(PSTR("eth rev: "));
  u08 rev = enc28j60_init(net_get_mac());
  uart_send_hex_byte_crlf(rev);
  // if no ethernet controller found then panic
  if(rev == 0) {
    uart_send_pstring(PSTR("NOT FOUND! PANIC!!\r\n"));
    // can't proceed :(
    while(1) {}
  }  
}

int main (void)
{
  init_hw();
  
  // send welcome
  uart_send_pstring(PSTR("\r\nWelcome to plipbox " VERSION "\r\n"));
  uart_send_pstring(PSTR("by lallafa (http://www.lallafa.de/blog)\r\n\r\n"));
  
  // init ethernet controller
  init_eth();

  // param init
  param_init();  
  param_dump();
  uart_send_crlf();

  eth_rx_init();
  eth_tx_init();
  
  // main loop
  while(1) {
    eth_state_worker();
    plip_state_worker();
    
    eth_rx_worker();
    eth_tx_worker();
    
    // small hack to enter commands
    if(uart_read_data_available()) {
      u08 cmd = uart_read();
      switch(cmd) {
        case ' ':
        {
          const u08 ip[4] = { 173,194,35,159 }; // google.de
          u08 result = eth_tx_send_ping_request(ip);
          uart_send_hex_byte_crlf(result);
          break;
        }
        case 'p':
        {
          const u08 ip[4] = { 192,168,2,20 }; // my box
          u08 result = eth_tx_send_ping_request(ip);
          uart_send_hex_byte_crlf(result);
          break;
        }
      }
    }
    
  }

  return 0;
} 
