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

static u08 update_proto_status(u08 status_changed, u08 new_status)
{
  u08 trigger = 0;

  // rx pending changed!
  if(status_changed & PROTO_CMD_STATUS_RX_PENDING) {
    // rx pending not yet set...
    if((proto_status & PROTO_CMD_STATUS_RX_PENDING) == 0) {
      proto_status |= PROTO_CMD_STATUS_RX_PENDING;
      trigger = 1;
      DT; DS("RX!"); DNL;
    }
    // rx pending is set
    else {
      proto_status &= ~PROTO_CMD_STATUS_RX_PENDING;
      trigger = 1;
      DT; DS("rx."); DNL;
    }
  }

  // link went up
  if(status_changed & PROTO_CMD_STATUS_LINK_UP) {
    // link not yet up
    if((proto_status & PROTO_CMD_STATUS_LINK_UP) == 0) {
      proto_status |= PROTO_CMD_STATUS_LINK_UP;
      trigger = 1;
      uart_send_time_stamp_spc();
      uart_send_pstring(PSTR("mode: link up!"));
      uart_send_crlf();
    }
    else {
      proto_status &= ~PROTO_CMD_STATUS_LINK_UP;
      trigger = 1;
      uart_send_time_stamp_spc();
      uart_send_pstring(PSTR("mode: link down!"));
      uart_send_crlf();
    }
  }

  return trigger;
}

static u08 poll_and_update_proto_status(void)
{
  // poll status from mode
  u08 new_status = mode_mod_poll_status();

  // check which status bits changed
  u08 status_change = new_status ^ proto_status;

  // update status change
  return update_proto_status(status_change, new_status);
}

void mode_handle(void)
{
  if(attached) {
    // if not in a tx/rx phase then check mode status, e.g. rx packet or link up/down
    if(proto_cmd_get_state() == PROTO_CMD_STATE_IDLE) {
      u08 trigger = poll_and_update_proto_status();
      if(trigger) {
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

  // attach and init status
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

    // set idle state but do NOT trigger a change since host will ignore it
    proto_status = PROTO_CMD_STATUS_IDLE;
  } else {
    uart_send_time_stamp_spc();
    uart_send_pstring(PSTR("mode: already detached!"));
    uart_send_crlf();
  }
}

void mode_ping(void)
{
  if(attached) {
    // if not in a tx/rx phase then check mode status, e.g. rx packet or link up/down
    if(proto_cmd_get_state() == PROTO_CMD_STATE_IDLE) {
      mode_mod_ping();
    }
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

u08 mode_tx_end(u16 size)
{
  if(attached) {
    u08 tx_status = mode_mod_tx_end(size);
    poll_and_update_proto_status();
    return tx_status | proto_status;
  } else {
    DS("tx_end"); not_attached();
    return 0;
  }
}

u08 mode_rx_size(u16 *got_size)
{
  if(attached) {
    return mode_mod_rx_size(got_size);
  } else {
    DS("rx_size"); not_attached();
    *got_size = 0;
    return MODE_ERROR;
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
    poll_and_update_proto_status();
    return rx_status | proto_status;
  } else {
    DS("rx_end"); not_attached();
    return 0;
  }
}
