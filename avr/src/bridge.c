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

#ifdef DEBUG_BRIDGE
#define DEBUG
#endif

#include "debug.h"
#include "uartutil.h"
#include "bridge.h"
#include "param.h"
#include "hw_timer.h"

#include "pkt_buf.h"
#include "pio_util.h"
#include "pio.h"

#include "proto_cmd.h"
#include "proto_cmd_shared.h"

#include "dump.h"

static u08 mode = PROTO_CMD_MODE_BRIDGE;
static u08 transfer = PROTO_CMD_MODE_BUF_TRANSFER;
static u16 loopback_timeout = 0;
static u16 loopback_size = 0;
static u08 loopback_trigger = 0;
static u08 status = 0;
static hw_timer_ms_t loopback_start = 0;

static u08 update_rx_pending_status(void)
{
  u08 num_pending = 0;

  // normal operation
  if(mode == PROTO_CMD_MODE_BRIDGE) {
    // check for pending packets and pio device
    num_pending = pio_has_recv();
  }
  // loopback operation
  else {
    if(loopback_trigger) {
      num_pending = 1;
    }
  }

  // rx pending not yet set...
  if((status & PROTO_CMD_STATUS_RX_PENDING) == 0) {
    // but pio has packets... so set flag
    if(num_pending > 0) {
      status |= PROTO_CMD_STATUS_RX_PENDING;
      DT; DS("RX++"); DNL;
      return 1;
    }
  }
  // rx pending is set
  else {
    // pio has no packets... clear flag
    if(num_pending == 0) {
      status &= ~PROTO_CMD_STATUS_RX_PENDING;
      DT; DS("RX--"); DNL;
    }
  }
  return 0;
}

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

// async poll of status
u16 proto_cmd_api_get_status(void)
{
  return status;
}

// ----- tx packet from parallel port to pio -----

u08 *proto_cmd_api_tx_begin(u16 size)
{
  // receive packet into pkt_buf
  return pkt_buf;
}

u08 proto_cmd_api_tx_end(u16 size)
{
#ifdef DEBUG_DUMP_FRAMES
  dump_eth_pkt(pkt_buf, size);
  uart_send_crlf();
  uart_send_hex_buf(0, pkt_buf, size);
#endif

  // send pkt_buf via pio
  u08 tx_status = PROTO_CMD_STATUS_IDLE;
  u08 pio_res = pio_util_send_packet(size);
  if(pio_res == PIO_OK) {
    // loopback?
    if(mode != PROTO_CMD_MODE_BRIDGE) {
      loopback_timeout = 200;
      loopback_start = hw_timer_millis();
      loopback_size = size;
    }
  } else {
    tx_status = PROTO_CMD_STATUS_TX_ERROR;
  }

  update_rx_pending_status();
  return status | tx_status;
}

// ----- rx packet from pio and send to parallel port -----

u08 proto_cmd_api_rx_size(u16 *size)
{
  u08 pio_res = PIO_OK;
  if(mode == PROTO_CMD_MODE_BRIDGE) {
    pio_res = pio_recv_size(size);
  } else {
    *size = loopback_size;
  }

  // clear loopback state
  loopback_trigger = 0;

  u08 rx_status = PROTO_CMD_STATUS_IDLE;
  if(pio_res != PIO_OK) {
    uart_send_pstring(PSTR("RX ERR!\r\n"));
    *size = 0;
    rx_status = PROTO_CMD_STATUS_RX_ERROR;
    update_rx_pending_status();
  }

  return rx_status | status;
}

u08 *proto_cmd_api_rx_begin(u16 size)
{
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
  update_rx_pending_status();
  return status;
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

void bridge_handle(void)
{
  // if not in a tx/rx phase then check for incoming packets
  if(proto_cmd_get_state() == PROTO_CMD_STATE_IDLE) {

    // handle loopback
    if(loopback_timeout > 0) {
      if(hw_timer_millis_timed_out(loopback_start, loopback_timeout)) {
        loopback_timeout = 0;
        loopback_trigger = 1;
        DT; DS("loop."); DNL;
      }
    }

    // check for rx pending
    u08 is_pending = update_rx_pending_status();
    if(is_pending) {
      // send async trigger to host -> will check status
      proto_cmd_trigger_status();
    }
  }
}
