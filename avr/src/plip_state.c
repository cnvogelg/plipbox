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
#include "timer.h"
#include "uartutil.h"

static u08 state = PLIP_STATE_OFFLINE;
static u16 my_timer;

static u08 time_passed(void)
{
  if((my_timer ^ timer_10ms) & PLIP_STATE_TIMER_MASK) {
    my_timer = timer_10ms;
    return 1;
  }
  return 0;
}

u08 plip_state_worker(void)
{
  if(time_passed()) {
    if(state == PLIP_STATE_OFFLINE) {
      u08 line = plip_get_line_status();
      if(line == PLIP_LINE_OK) {
        uart_send_pstring(PSTR("plip: online\r\n"));
        state = PLIP_STATE_ONLINE;
      }
    }
    else {
      u08 line = plip_get_line_status();
      if(line == PLIP_LINE_OFF) {
        uart_send_pstring(PSTR("plip: power off\r\n"));
        state = PLIP_STATE_OFFLINE;
      } else if(line == PLIP_LINE_DISABLED) {
        uart_send_pstring(PSTR("plip: offline\r\n"));
        state = PLIP_STATE_OFFLINE;
      }
    }
  }
  return state;
}
