/*
 * plip_state.c: handle state of plip device
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

#include "plip_state.h"
#include "plip.h"
#include "plip_rx.h"
#include "timer.h"
#include "uartutil.h"

static u08 state = PLIP_STATE_NO_AMIGA;
static u16 my_timer;

static u08 time_passed(void)
{
  if((my_timer ^ timer_10ms) & PLIP_STATE_TIMER_MASK) {
    my_timer = timer_10ms;
    return 1;
  }
  return 0;
}

static void check_lost_line(void)
{
  u08 line = plip_get_line_status();
  if(line == PLIP_LINE_OFF) {
    state = PLIP_STATE_LOST_AMIGA;
  } else if(line == PLIP_LINE_DISABLED) {
    state = PLIP_STATE_LINK_DOWN;
  }
}

u08 plip_state_worker(void)
{
  u08 line;
  
  switch(state) {
    case PLIP_STATE_NO_AMIGA: // amiga powered off
      line = plip_get_line_status();
      if(line != PLIP_LINE_OFF) {
        state = PLIP_STATE_GOT_AMIGA;
      }
      break;
    case PLIP_STATE_GOT_AMIGA:
      uart_send_pstring(PSTR("plip: got amiga\r\n"));
      state = PLIP_STATE_OFFLINE;
      break;
    case PLIP_STATE_OFFLINE: // amiga powered on, but driver not active
      line = plip_get_line_status();
      if(line == PLIP_LINE_OFF) {
        state = PLIP_STATE_LOST_AMIGA;
      } else if(line == PLIP_LINE_OK) {
        state = PLIP_STATE_LINK_UP;
      }
      break;
    case PLIP_STATE_LINK_UP:
      uart_send_pstring(PSTR("plip: link up\r\n"));
      state = PLIP_STATE_WAIT_CONFIG;
      break;
    case PLIP_STATE_WAIT_CONFIG: // driver active but no config yet
      if(plip_rx_is_configured()) {
        state = PLIP_STATE_CONFIGURED;
      }
      check_lost_line();
      break;
    case PLIP_STATE_CONFIGURED:
      uart_send_pstring(PSTR("plip: configured\r\n"));
      state = PLIP_STATE_ONLINE;
      break;
    case PLIP_STATE_ONLINE: // driver active and operational
      if(time_passed()) {
        check_lost_line();
      }
      break;
    case PLIP_STATE_LINK_DOWN:
      uart_send_pstring(PSTR("plip: link down\r\n"));
      state = PLIP_STATE_OFFLINE;
      break;
    case PLIP_STATE_LOST_AMIGA:
      uart_send_pstring(PSTR("plip: lost amiga\r\n"));
      state = PLIP_STATE_NO_AMIGA;
      break;
  }
  return state;
}
