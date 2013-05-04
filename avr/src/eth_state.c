/*
 * eth_tx.c: handle state of ethernet adapter
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

#include "eth_state.h"

#include "timer.h"
#include "uartutil.h"
#include "uart.h"
#include "enc28j60.h"

static u08 state = ETH_STATE_LINK_DOWN;
static u16 my_timer;

static u08 time_passed(void)
{
  if((my_timer ^ timer_10ms) & ETH_STATE_TIMER_MASK) {
    my_timer = timer_10ms;
    return 1;
  }
  return 0;
}

u08 eth_state_worker(u08 plip_online)
{
  switch(state) {
    case ETH_STATE_LINK_DOWN:
      if(time_passed()) {
        // check if eth adapter is online
        if(enc28j60_is_link_up()) {
          state = ETH_STATE_LINK_UP;
          uart_send_time_stamp_spc();
          uart_send_pstring(PSTR(" eth: link up\r\n"));
        }
      }
      break;
    case ETH_STATE_LINK_UP:
      if(time_passed()) {
        // check if eth adapter went offline
        if(!enc28j60_is_link_up()) {
          state = ETH_STATE_LINK_DOWN;
          uart_send_time_stamp_spc();
          uart_send_pstring(PSTR(" eth: link down\r\n"));
        }
      }  
      break;
  }
  return state;
}
    