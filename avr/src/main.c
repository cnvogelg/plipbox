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

#include "global.h"
#include "board.h"
#include "uart.h"
#include "uartutil.h"
#include "timer.h"
#include "par_low.h"
#include "param.h"
#include "spi.h"
#include "enc28j60.h"
#include "cmd.h"

#include "net/net.h"
#include "eth_rx.h"
#include "plip_rx.h"
#include "plip_tx.h"
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

int main (void)
{
  init_hw();
  
  // send welcome
  uart_send_pstring(PSTR("\r\nWelcome to plipbox " VERSION " " BUILD_DATE "\r\n"));
  uart_send_pstring(PSTR("by lallafa (http://www.lallafa.de/blog)\r\n\r\n"));
  
  // param init
  param_init();  
  param_dump();
  uart_send_crlf();

  eth_rx_init();
  plip_rx_init();
  plip_tx_init();
  
  // setup ethernet
  u08 rev = enc28j60_init(param.mac_addr);
  if(rev == 0) {
    uart_send_pstring(PSTR("enc28j60: ERROR SETTING UP!!\r\n"));
    while(1) {}
  } else {
    uart_send_pstring(PSTR("enc28j60: rev="));
    uart_send_hex_byte(rev);
    uart_send_crlf();
  }
    
  // main loop
  while(1) {
    u08 plip_state = plip_state_worker();
    u08 plip_online = (plip_state == PLIP_STATE_LINK_UP);
    u08 eth_state = eth_state_worker(plip_online);
    u08 eth_online = (eth_state == ETH_STATE_LINK_UP);
    
    eth_rx_worker(eth_state, plip_online);
    plip_rx_worker(plip_state, eth_online);
    plip_tx_worker(plip_online);
    
    cmd_worker();
  }

  return 0;
} 
