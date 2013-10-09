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

#include "pb_state.h"
#include "pb_proto.h"
#include "timer.h"
#include "uartutil.h"

static u08 state = PB_STATE_LINK_DOWN;
static u16 my_timer;
static u08 count_down;

static u08 time_passed(void)
{
  if((my_timer ^ timer_10ms) & PB_STATE_TIMER_MASK) {
    my_timer = timer_10ms;
    return 1;
  }
  return 0;
}

u08 pb_state_worker(void)
{
  switch(state) {
    case PB_STATE_LINK_DOWN:
      if(time_passed()) {
        u08 line = pb_proto_get_line_status();
        if(line == PBPROTO_LINE_OK) {
          state = PB_STATE_LINK_UP;
          uart_send_time_stamp_spc();
          uart_send_pstring(PSTR("plip: link up\r\n"));
        }
      }
      break;
    case PB_STATE_LINK_UP:
      if(time_passed()) {
        u08 line = pb_proto_get_line_status();
        if(line != PBPROTO_LINE_OK) {
          count_down ++;
          if(count_down == 5) {
            state = PB_STATE_LINK_DOWN;
            uart_send_time_stamp_spc();
            uart_send_pstring(PSTR("plip: link down\r\n"));
          }
        } else {
          count_down = 0;
        }
      }
      break;
  }
  return state;
}
