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
#include "param.h"

#include "pkt_buf.h"
#include "pio_util.h"
#include "pio.h"

#include "proto_cmd.h"
#include "proto_cmd_shared.h"

#include "dump.h"

static u08 mode = PROTO_CMD_MODE_BRIDGE;
static u08 transfer = PROTO_CMD_MODE_BUF_TRANSFER;
static u08 rx_notified = 0;
static u08 tx_done = 0;
static u16 loopback_size = 0;
static u08 status = 0;

// ----- implement API of proto_cmd -----

void proto_cmd_api_attach(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("proto: attach"));
  uart_send_crlf();

  status |= PROTO_CMD_STATUS_ATTACHED;
}

void proto_cmd_api_detach(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("proto: detach"));
  uart_send_crlf();

  status &= ~PROTO_CMD_STATUS_ATTACHED;
}

u16 proto_cmd_api_get_status(void)
{
  return status;
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

#ifdef DEBUG_DUMP_FRAMES
  dump_eth_pkt(pkt_buf, size);
  uart_send_crlf();
  uart_send_hex_buf(0, pkt_buf, size);
#endif

  // send pkt_buf via pio
  u08 pio_res = pio_util_send_packet(size);
  if(pio_res == PIO_OK) {

    // loopback?
    if(mode != PROTO_CMD_MODE_BRIDGE) {
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
  if(mode == PROTO_CMD_MODE_BRIDGE) {
    pio_res = pio_recv_size(size);
  } else {
    *size = loopback_size;
  }
  DW(*size);
  DNL;

  // reset rx notified
  rx_notified = 0;

  // clear status mask
  status &= ~PROTO_CMD_STATUS_RX_PENDING;

  if(pio_res != PIO_OK) {
    return PROTO_CMD_RESULT_ERROR;
  }
  return PROTO_CMD_RESULT_OK;
}

u08 *proto_cmd_api_rx_begin(u16 size)
{
  DS("rx begin");
  DNL;

  if(mode == PROTO_CMD_MODE_BRIDGE) {
    pio_util_recv_packet(size);
  }

#ifdef DEBUG_DUMP_FRAMES
  dump_eth_pkt(pkt_buf, size);
  uart_send_crlf();
  uart_send_hex_buf(0, pkt_buf, size);
#endif

  return pkt_buf;
}

u08 proto_cmd_api_rx_end(u16 size)
{
  DS("rx end");
  DNL;

  return PROTO_CMD_RESULT_OK;
}

// ----- proto_cmd config -----

void proto_cmd_api_set_mode(u16 new_mode)
{
  // make sure we are not attached
  if(status & PROTO_CMD_STATUS_ATTACHED) {
    return;
  }

  mode = new_mode & PROTO_CMD_MODE_MASK;
  transfer = new_mode & PROTO_CMD_MODE_TRANSFER_MASK;
}

u16 proto_cmd_api_get_mode(void)
{
  return mode;
}

void proto_cmd_api_set_mac(mac_t mac)
{
  param_set_mac(mac);
  pio_set_mac(mac);
}

void proto_cmd_api_get_mac(mac_t mac)
{
  param_get_mac(mac);
}

void proto_cmd_api_get_def_mac(mac_t mac)
{
  param_get_def_mac(mac);
}

// ----- handler -----


void bridge_init(u08 pio_ok)
{
  // set status flag
  if(pio_ok) {
    status |= PROTO_CMD_STATUS_HW_INIT;
  }

  // setup pio mac
  pio_set_mac(param.mac_addr);
  pio_enable_rx();
}

static u08 has_packet(void)
{
  // normal operation
  if(mode == PROTO_CMD_MODE_BRIDGE) {
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
    status |= PROTO_CMD_STATUS_RX_PENDING;
    proto_cmd_trigger_status();
    rx_notified = 1;
  }
}
