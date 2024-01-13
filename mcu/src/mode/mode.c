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
#include "proto_cmd.h"
#include "proto_cmd_shared.h"
#include "param.h"
#include "uartutil.h"

static u08 proto_status;
static u08 attached;

void mode_init(void)
{
  proto_status = PROTO_CMD_STATUS_IDLE;
  attached = 0;
  mode_mod_init();
}

u08 mode_get_proto_status(void)
{
  return proto_status;
}

static u08 update_rx_pending_state(void)
{
  u08 res = mode_mod_rx_poll();

  // rx pending not yet set...
  if((proto_status & PROTO_CMD_STATUS_RX_PENDING) == 0) {
    if(res == MODE_RX_PENDING) {
      proto_status |= PROTO_CMD_STATUS_RX_PENDING;
      DT; DS("RX++"); DNL;
      return 1;
    }
  }
  // rx pending is set
  else {
    // pio has no packets... clear flag
    if(res != MODE_RX_PENDING) {
      proto_status &= ~PROTO_CMD_STATUS_RX_PENDING;
      DT; DS("RX--"); DNL;
    }
  }
  return 0;
}

void mode_handle(void)
{
  if(attached) {
    // if not in a tx/rx phase then check for incoming packets
    if(proto_cmd_get_state() == PROTO_CMD_STATE_IDLE) {
      u08 became_pending = update_rx_pending_state();
      if(became_pending) {
          // send async trigger to host -> will check status
          proto_cmd_trigger_status();
      }
    }
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

  u08 result = mode_mod_attach();
  if(result == MODE_OK) {
    attached = 1;
    proto_status = PROTO_CMD_STATUS_ATTACHED;
  } else {
    proto_status = PROTO_CMD_STATUS_INIT_ERROR;
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

  } else {
    DS("detach: "); not_attached();
  }

  proto_status = PROTO_CMD_STATUS_IDLE;
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

u08 mode_tx_end(u16 size)
{
  if(attached) {
    u08 tx_status = mode_mod_tx_end(size);
    u08 result = 0;
    if(tx_status != MODE_OK) {
      result = PROTO_CMD_STATUS_TX_ERROR;
    }
    update_rx_pending_state();
    return proto_status | result;
  } else {
    DS("tx_end"); not_attached();
    return 0;
  }
}

u16 mode_rx_size(void)
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

u08 mode_rx_end(u16 size)
{
  if(attached) {
    u08 rx_status = mode_mod_rx_end(size);
    u08 result = 0;
    if(rx_status != MODE_OK) {
      result = PROTO_CMD_STATUS_RX_ERROR;
    }
    update_rx_pending_state();
    return proto_status | result;
  } else {
    DS("rx_end"); not_attached();
    return 0;
  }
}
