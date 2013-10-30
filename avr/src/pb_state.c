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
#include "pb_io.h"
#include "timer.h"
#include "uartutil.h"

static u08 state = PB_STATE_LINK_DOWN;

u08 pb_state_worker(void)
{
  switch(state) {
    case PB_STATE_LINK_DOWN:
      if(sana_online) {
        uart_send_time_stamp_spc();
        uart_send_pstring(PSTR("pbp: link up\r\n"));
        state = PB_STATE_LINK_UP;
      }
      break;
    case PB_STATE_LINK_UP:
      if(!sana_online) {
        uart_send_time_stamp_spc();
        uart_send_pstring(PSTR("pbp: link down\r\n"));
        state = PB_STATE_LINK_DOWN;
      }
      break;
  }
  return state;
}
