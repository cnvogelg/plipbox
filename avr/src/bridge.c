/*
 * bridge.c: bridge packets from plip to eth and back
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

#include "global.h"

#include "debug.h"
#include "uartutil.h"
#include "bridge.h"

#include "pkt_buf.h"
#include "pio_util.h"
#include "pio.h"

#include "proto_cmd.h"
#include "proto_cmd_shared.h"

static u08 mode = BRIDGE_MODE_LOOPBACK_BUF;
static u08 transfer = BRIDGE_TRANSFER_BUF;
static u08 rx_notified = 0;
static u08 tx_done = 0;
static u16 loopback_size = 0;

// ----- implement API of proto_cmd -----

void proto_cmd_api_attach(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("attach"));
  uart_send_crlf();

  proto_cmd_set_status_mask(PROTO_CMD_STATUS_ATTACHED, 0);
}

void proto_cmd_api_detach(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("detach"));
  uart_send_crlf();

  proto_cmd_clr_status_mask(PROTO_CMD_STATUS_ATTACHED, 0);
}

// ----- tx packet from parallel port to pio -----

u08 *proto_cmd_api_tx_begin(u16 size)
{
  DS("tx begin ");
  DW(size);
  DNL;

  // receive packet into pkt_buf
  return pkt_buf;
}

u08 proto_cmd_api_tx_end(u16 size)
{
  DS("tx end");
  DNL;

  // send pkt_buf via pio
  u08 pio_res = pio_util_send_packet(size);
  if(pio_res == PIO_OK) {

    // loopback?
    if(mode != BRIDGE_MODE_FORWARD) {
      tx_done = 1;
      loopback_size = size;
    }

    return PROTO_CMD_RESULT_OK;
  } else {
    return PROTO_CMD_RESULT_ERROR;
  }
}

// ----- rx packet from pio and send to parallel port -----

u08 proto_cmd_api_rx_size(u16 *size)
{
  u08 pio_res = PIO_OK;
  DS("rx size ");
  if(mode == BRIDGE_MODE_FORWARD) {
    pio_res = pio_recv_size(size);
  } else {
    *size = loopback_size;
  }
  DW(*size);
  DNL;

  // reset rx notified
  rx_notified = 0;

  // clear status mask
  proto_cmd_clr_status_mask(PROTO_CMD_STATUS_RX_PENDING, 0);

  if(pio_res != PIO_OK) {
    return PROTO_CMD_RESULT_ERROR;
  }
  return PROTO_CMD_RESULT_OK;
}

u08 *proto_cmd_api_rx_begin(u16 size)
{
  DS("rx begin");
  DNL;

  if(mode == BRIDGE_MODE_FORWARD) {
    pio_util_recv_packet(size);
  }

  return pkt_buf;
}

u08 proto_cmd_api_rx_end(u16 size)
{
  DS("rx end");
  DNL;

  return PROTO_CMD_RESULT_OK;
}

// ----- handler -----

u08 bridge_get_mode(void)
{
  return mode;
}

void bridge_set_mode(u08 new_mode)
{
  mode = new_mode;
}

u08 bridge_get_transfer(void)
{
  return transfer;
}

void bridge_set_transfer(u08 new_transfer)
{
  transfer = new_transfer;
}

void bridge_init(u08 pio_ok)
{
  // set status flag
  proto_cmd_set_status_mask(PROTO_CMD_STATUS_HW_INIT, 0);
}

static u08 has_packet(void)
{
  // normal operation
  if(mode == BRIDGE_MODE_FORWARD) {
    // check for pending packets and pio device
    u08 n = pio_has_recv();
    return (n > 0);
  }
  // loopback operation
  else {
    if(tx_done) {
      tx_done = 0;
      return 1;
    } else {
      return 0;
    }
  }
}

void bridge_handle(void)
{
  if(!rx_notified && has_packet()) {
    DS("RX_PENDING!"); DNL;
    proto_cmd_set_status_mask(PROTO_CMD_STATUS_RX_PENDING, 1);
    rx_notified = 1;
  }
}
