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

#include "net/net.h"
#include "net/arp.h"

#ifdef HAVE_NAT
#include "net/arp_cache.h"
#endif

#include "enc28j60.h"
#include "timer.h"
#include "uartutil.h"
#include "uart.h"
#include "pkt_buf.h"

static u08 state = ETH_STATE_DISABLED;
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
  u08 rev;
  
  switch(state) {
    case ETH_STATE_DISABLED:
      // if plip is online then setup ethernet
      if(plip_online) {
        state = ETH_STATE_INIT;
      }
      break;
    case ETH_STATE_INIT:
      uart_send_pstring(PSTR(" eth: init\r\n"));
      rev = enc28j60_init(net_get_mac());
      if(rev != 0) {
        state = ETH_STATE_OFFLINE;
      } else {
        state = ETH_STATE_RETRY_INIT;
      }
      break;
    case ETH_STATE_RETRY_INIT:
      if(time_passed()) {
        state = ETH_STATE_INIT;
      }  
      break;
    case ETH_STATE_OFFLINE:
      if(time_passed()) {
        // check if eth adapter is online
        if(enc28j60_is_link_up()) {
          state = ETH_STATE_LINK_UP;
        }
      }
      if(!plip_online) {
        state = ETH_STATE_EXIT;
      }
      break;
    case ETH_STATE_LINK_UP:
      uart_send_pstring(PSTR(" eth: link up\r\n"));
      state = ETH_STATE_ONLINE;
      break;
    case ETH_STATE_ONLINE:
      if(time_passed()) {
        // check if eth adapter went offline
        if(!enc28j60_is_link_up()) {
          state = ETH_STATE_LINK_DOWN;
        }
      }  
      if(!plip_online) {
        state = ETH_STATE_EXIT;
      }
      break;
    case ETH_STATE_LINK_DOWN:
      uart_send_pstring(PSTR(" eth: link down\r\n"));
      state = ETH_STATE_OFFLINE;
      break;
    case ETH_STATE_EXIT:
      uart_send_pstring(PSTR(" eth: exit\r\n"));
      state = ETH_STATE_DISABLED;
      enc28j60_power_down();
      break;
  }
  return state;
}
    