/*
 * eth_tx.c: handle state of ethernet adapter
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

#include "eth_state.h"

#include "net/net.h"
#include "net/arp.h"
#include "net/arp_cache.h"

#include "enc28j60.h"
#include "timer.h"
#include "uartutil.h"
#include "uart.h"
#include "pkt_buf.h"

static u08 state = ETH_STATE_OFFLINE;
static u16 my_timer;

static u08 time_passed(void)
{
  if((my_timer ^ timer_10ms) & ETH_STATE_TIMER_MASK) {
    my_timer = timer_10ms;
    return 1;
  }
  return 0;
}

static u08 link_up(void)
{
  return ETH_STATE_WAIT_ARP;
}

static u08 wait_arp(void)
{
  if(arp_cache_get_gw_mac() == 0) {
    return ETH_STATE_WAIT_ARP;
  } else {
    return ETH_STATE_START_NET;
  }
}

u08 eth_state_worker(void)
{
  switch(state) {
    case ETH_STATE_OFFLINE:
      if(time_passed()) {
        // check if eth adapter is online
        if(enc28j60_is_link_up()) {
          state = ETH_STATE_LINK_UP;
        }
      }
      break;
      
    case ETH_STATE_LINK_UP:
      uart_send_pstring(PSTR("eth: link up\r\n"));
      state = link_up();
      break;
    case ETH_STATE_WAIT_ARP:
      if(!enc28j60_is_link_up()) {
        state = ETH_STATE_LINK_DOWN;
      } else {
        state = wait_arp();
      }
      break;
    case ETH_STATE_START_NET:
      uart_send_pstring(PSTR("eth: start net\r\n"));
      state = ETH_STATE_ONLINE;
      break;
      
    case ETH_STATE_ONLINE:
      if(time_passed()) {
        // check if eth adapter went offline
        if(!enc28j60_is_link_up()) {
          state = ETH_STATE_STOP_NET;
        }
      }  
      break;
    
    case ETH_STATE_STOP_NET:
      uart_send_pstring(PSTR("eth: stop net\r\n"));
      state = ETH_STATE_LINK_DOWN;
      break;
    case ETH_STATE_LINK_DOWN:
      uart_send_pstring(PSTR("eth: link down\r\n"));
      state = ETH_STATE_OFFLINE;
      break;
  }
  return state;
}
    