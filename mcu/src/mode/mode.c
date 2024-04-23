/*
 * mode.c - plipbox operation mode
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

#include "types.h"

#ifdef DEBUG_MODE
#define DEBUG
#endif

#include "debug.h"
#include "mode.h"
#include "mode_mod.h"
#include "mode_cmd.h"
#include "param.h"
#include "uartutil.h"

static u08 attached;

void mode_init(void)
{
  attached = 0;
  mode_mod_init();
  mode_cmd_init();
}

void mode_dump_modes(void)
{
  uart_send_pstring(PSTR("Modes:"));
  uart_send_crlf();

  u08 num = mode_mod_get_num_modes();
  for(u08 i=0;i<num;i++) {
    uart_send('#');
    uart_send_hex_byte(i);
    uart_send_spc();

    u32 tag = mode_mod_tag_at(i);
    uart_send_tag(tag);
    uart_send_spc();

    uart_send_spc();
    const char *name = mode_mod_name_at(i);
    uart_send_pstring(name);

    uart_send_crlf();
  }
  uart_send_crlf();
}

u08 mode_get_num_modes(void)
{
  return mode_mod_get_num_modes();
}

u08 mode_find_tag(u32 tag)
{
  u08 num = mode_get_num_modes();
  for(u08 i=0;i<num;i++) {
    u32 mtag = mode_mod_tag_at(i);
    if(mtag == tag) {
      return i;
    }
  }
  return MODE_ID_INVALID;
}

void mode_get_def(u08 index, mode_def_t *def)
{
  mode_mod_def_at(index, def);
}

// ----- mode functions -----

void mode_work(void)
{
  if(attached) {
    // let the current mode work
    mode_mod_work();

    // if a trigger is set then raise it
    // so the host can read out the event mask and other regs
    mode_cmd_process_trigger();
  }
}

void mode_attach(void)
{
  if(attached) {
    uart_send_time_stamp_spc();
    uart_send_pstring(PSTR("mode: already attached!"));
    uart_send_crlf();
    return;
  }

  // get current module index from param
  u08 mod_index = param_get_mode();
  mode_mod_set_current(mod_index);

  // attach and init status
  u08 result = mode_mod_attach();
  if(result == MODE_OK) {
    attached = 1;
    mode_cmd_set_hw_status(PROTO_STATUS_HW_ON);
  } else {
    mode_cmd_set_hw_status(PROTO_STATUS_HW_ERROR_INIT);
  }

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("mode: #"));
  uart_send_hex_byte(mode_mod_get_current());
  uart_send_spc();
  uart_send_pstring(mode_mod_name());
  uart_send_pstring(PSTR(" -> "));
  uart_send_hex_byte(result);
  uart_send_crlf();
}

static void not_attached(void)
{
  DS(": not attached!!"); DNL;
}

void mode_detach(void)
{
  if(attached) {
    attached = 0;
    mode_mod_detach();

    uart_send_time_stamp_spc();
    uart_send_pstring(PSTR("mode: detached."));
    uart_send_crlf();

    mode_cmd_set_hw_status(PROTO_STATUS_HW_OFF);
  } else {
    uart_send_time_stamp_spc();
    uart_send_pstring(PSTR("mode: already detached!"));
    uart_send_crlf();
  }
}

void mode_ping(void)
{
  if(attached) {
    mode_mod_ping();
  }
}

u08 *mode_tx_begin(u16 size)
{
  if(attached) {
    return mode_mod_tx_begin(size);
  } else {
    DS("tx_begin"); not_attached();
    return 0;
  }
}

u16 mode_tx_end(u16 size)
{
  if(attached) {
    u16 tx_error = mode_mod_tx_end(size);
    mode_cmd_set_tx_error(tx_error);
    // let the mode work so an rx_pending might be set
    mode_mod_work();
    // take current event mask and return now
    u16 event_mask = mode_cmd_take_event_mask();
    return event_mask;
  } else {
    DS("tx_end"); not_attached();
    return 0;
  }
}

u16 mode_rx_size()
{
  if(attached) {
    return mode_mod_rx_size();
  } else {
    DS("rx_size"); not_attached();
    return 0;
  }
}

u08 *mode_rx_begin(u16 size)
{
  if(attached) {
    return mode_mod_rx_begin(size);
  } else {
    DS("rx_begin"); not_attached();
    return 0;
  }
}

u16 mode_rx_end(u16 size)
{
  if(attached) {
    u16 rx_error = mode_mod_rx_end(size);
    mode_cmd_set_rx_error(rx_error);
    // let the mode work so an rx_pending might be set
    mode_mod_work();
    // take current event mask and return now
    u16 event_mask = mode_cmd_take_event_mask();
    return event_mask;
  } else {
    DS("rx_end"); not_attached();
    return 0;
  }
}
