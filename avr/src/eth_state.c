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
#include "pb_io.h"

static u08 state = ETH_STATE_LINK_DOWN;

u08 eth_state_worker(u08 plip_online)
{
  switch(state) {
    case ETH_STATE_LINK_DOWN:
      if(plip_online) {
        enc28j60_start(sana_mac);
        
        uart_send_time_stamp_spc();
        uart_send_pstring(PSTR("eth: link up\r\n"));
        state = ETH_STATE_LINK_UP;
      }
      break;
    case ETH_STATE_LINK_UP:
      if(!plip_online) {
        enc28j60_stop();
  
        uart_send_time_stamp_spc();
        uart_send_pstring(PSTR("eth: link down\r\n"));
        state = ETH_STATE_LINK_DOWN;
      }  
      break;
  }
  return state;
}
    