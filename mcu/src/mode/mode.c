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
#include "param.h"
#include "uartutil.h"
#include "proto_api.h"

static u08 request_mode;
static u08 attached_mode;
static u08 attached;
static u08 status;

void mode_init(void)
{
  request_mode = MODE_FROM_PARAM;
  attached_mode = MODE_NONE;
  attached = 0;
  status = MODE_STATUS_DETACHED;

  mode_mod_init();
}

// ----- mode module query -----

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
    proto_api_process_trigger();
  }
}

u08 mode_attach(void)
{
  if(attached) {
    uart_send_time_stamp_spc();
    uart_send_pstring(PSTR("mode: already attached!"));
    uart_send_crlf();
    status = MODE_STATUS_ERROR_ALREADY_ATTACHED;
    return status;
  }

  // select mode
  u08 mode = request_mode;

  // pick mode from param?
  if(mode == MODE_FROM_PARAM) {
    mode = param_get_mode();
  }

  // valid mode?
  if(mode > mode_mod_get_num_modes()) {
    uart_send_time_stamp_spc();
    uart_send_pstring(PSTR("mode: invalid mode: "));
    uart_send_hex_byte(mode);
    uart_send_crlf();
    status = MODE_STATUS_ERROR_INVALID_MODE;
  }

  // get current module index from param
  mode_mod_set_current(mode);

  // attach and init status
  status = mode_mod_attach();
  if(status == MODE_STATUS_ATTACHED) {
    attached = 1;
    attached_mode = mode;
  }

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("mode: #"));
  uart_send_hex_byte(mode_mod_get_current());
  uart_send_spc();
  uart_send_pstring(mode_mod_name());
  uart_send_pstring(PSTR(" -> "));
  uart_send_hex_byte(status);
  uart_send_crlf();

  return status;
}

u08 mode_get_attached_mode(void)
{
  if(attached) {
    return attached_mode;
  } else {
    return MODE_NONE;
  }
}

void mode_set_request_mode(u08 mode)
{
  request_mode = mode;
}

static void not_attached(void)
{
  DS(": not attached!!"); DNL;
}

u08 mode_detach(void)
{
  if(attached) {
    attached = 0;
    attached_mode = MODE_NONE;
    status = MODE_STATUS_DETACHED;
    mode_mod_detach();

    uart_send_time_stamp_spc();
    uart_send_pstring(PSTR("mode: detached."));
    uart_send_crlf();
  } else {
    status = MODE_STATUS_ERROR_ALREADY_DETACHED;

    uart_send_time_stamp_spc();
    uart_send_pstring(PSTR("mode: already detached!"));
    uart_send_crlf();
  }

  return status;
}

void mode_ping(void)
{
  if(attached) {
    mode_mod_ping();
  }
}

// ----- mode packet I/O -----

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
    proto_api_set_tx_error(tx_error);
    // let the mode work so an rx_pending might be set
    mode_mod_work();
    // take current event mask and return now
    u16 event_mask = proto_api_take_event_mask();
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
    proto_api_set_rx_error(rx_error);
    // let the mode work so an rx_pending might be set
    mode_mod_work();
    // take current event mask and return now
    u16 event_mask = proto_api_take_event_mask();
    return event_mask;
  } else {
    DS("rx_end"); not_attached();
    return 0;
  }
}
