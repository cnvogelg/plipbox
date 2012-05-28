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
#include "slip.h"
#include "uartutil.h"
#include "stats.h"
#include "log.h"
#include "error.h"
	 
   // eth
#include "eth_rx.h"
#include "eth_tx.h"

#include "transfer.h"
#include "ping_plip.h"
#include "ping_slip.h"
#include "only_plip_rx.h"
#include "only_slip_rx.h"

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
  
  // enable uart
  uart_start_reception();

  // send welcome
#ifdef HAVE_SLIP
  // encapsulated into an invalid SLIP packet to not disturb an attached SLIP
  slip_send_end();
  uart_send_string("plip2slip: ");
  uart_send_hex_byte_crlf(param.mode);
  uart_send_hex_byte_crlf(rev);
  slip_send_end();
  
  // main loop
  while(1) {
    // reset stats & log
    stats_reset();
    log_init();
    error_init();
    
    // now enter mode loop
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
      case PARAM_MODE_ONLY_PLIP_RX:
        only_plip_rx_loop();
        break;
      case PARAM_MODE_ONLY_SLIP_RX:
        only_slip_rx_loop();
        break;
    }
  }
#else
  /* ----- network stuff ----- */
  uart_send_string("plip2eth: ");
  
  eth_rx_init();
  eth_tx_init();
  
  while(1) {
    eth_rx_worker();
    eth_tx_worker();
    
    // small hack to enter commands
    if(uart_read_data_available()) {
      u08 cmd;
      uart_read(&cmd);
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
#endif

  return 0;
} 
