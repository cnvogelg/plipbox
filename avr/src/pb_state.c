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

static u08 state = PB_STATE_DETECT_DRIVER;
static u08 retries;

u08 pb_state_worker(u08 last_io_state)
{
  switch(state) {

    /* check if device is online by watching the parallel port */
    case PB_STATE_DETECT_DRIVER:
      {
        u08 val = pb_proto_get_line_status();
        if(val == PBPROTO_LINE_OK) {
          uart_send_time_stamp_spc();
          uart_send_pstring(PSTR("pbs: device detected\r\n"));
          state = PB_STATE_SEEK_ONLINE;
          timer_10ms = 0;
          retries = 5;
        }
      }
      break;

    /* we have a driver and wait for magic packet from device */
    case PB_STATE_SEEK_ONLINE:
      {
        /* if some error happened then fall back to driver check */
        if(last_io_state == PB_IO_ERROR) {
          retries--;
          if(retries == 0) {
            uart_send_time_stamp_spc();
            uart_send_pstring(PSTR("pbs: error\r\n"));
            state = PB_STATE_DETECT_DRIVER;
          }
        }

        /* if magic packet was sent then we are online */
        if(sana_online) {
          uart_send_time_stamp_spc();
          uart_send_pstring(PSTR("pbs: device online\r\n"));
          state = PB_STATE_ONLINE;
        }

        /* request magic after a delay of 1s */
        if(timer_10ms > 100) {
          uart_send_time_stamp_spc();
          uart_send_pstring(PSTR("pbs: request magic\r\n"));
          pb_io_send_magic(PB_IO_MAGIC_ONLINE, 0);
          timer_10ms = 0;
        }
      }
      break;

    /* we got a valid magic packet and assume that the driver is operational */
    case PB_STATE_ONLINE:
      {
        /* if some error happened then fall back to seek */
        if(last_io_state == PB_IO_ERROR) {
          uart_send_time_stamp_spc();
          uart_send_pstring(PSTR("pbs: error\r\n"));
          state = PB_STATE_DETECT_DRIVER;
        }

        /* if a magic offline was received then driver was shut down regularly */
        if(!sana_online) {
          uart_send_time_stamp_spc();
          uart_send_pstring(PSTR("pbs: device offline\r\n"));
          state = PB_STATE_OFFLINE;          
        }
      }
      break;

    /* driver went away. wait for its return */
    case PB_STATE_OFFLINE:
      {
        /* if some error happened then fall back to seek */
        if(last_io_state == PB_IO_ERROR) {
          uart_send_time_stamp_spc();
          uart_send_pstring(PSTR("pbs: error\r\n"));
          state = PB_STATE_DETECT_DRIVER;
        }

        /* driver came back */
        if(sana_online) {
          uart_send_time_stamp_spc();
          uart_send_pstring(PSTR("pbs: device online\r\n"));
          state = PB_STATE_ONLINE;          
        }
      }
      break;

  }
  return state;
}
