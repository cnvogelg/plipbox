/*
 * nic.c - API for the network interface connector
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

#include "arch.h"
#include "types.h"

#include "nic.h"
#include "nic_mod.h"

#include "uartutil.h"
#include "param.h"

static u08 is_direct_io;
static u08 is_attached;
static u16 caps_in_use;

void nic_init(void)
{
  nic_mod_init();
  is_attached = 0;
}

void nic_set_device(u08 device)
{
  nic_mod_set_current(device);
}

u08 nic_attach_params(void)
{
  u08 nic = param_get_nic();
  u16 nic_caps = param_get_nic_caps();
  u08 nic_port = param_get_nic_port();
  mac_t mac;
  param_get_cur_mac(mac);

  nic_set_device(nic);
  u08 res = nic_attach(nic_caps, nic_port, mac);

  return res;
}

u08 nic_attach(u16 caps, u08 port, mac_t mac)
{
  u08 device = nic_mod_get_current();

  // get caps device has to offer
  u16 caps_available = nic_caps_available();

  // show hello
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("nic_attach: #"));
  // dev no
  uart_send_hex_byte(device);
  uart_send_spc();
  // dev name
  rom_pchar name = nic_mod_name();
  uart_send_pstring(name);
  // caps
  uart_send_pstring(PSTR(" caps="));
  uart_send_hex_word(caps);
  uart_send_pstring(PSTR("/"));
  uart_send_hex_word(caps_available);
  // port
  uart_send_pstring(PSTR(" port="));
  uart_send_hex_byte(port);
  // mac
  uart_send_pstring(PSTR(" mac="));
  uart_send_hex_mac(mac);

  // the caps we can use
  u16 caps_req = caps & caps_available;

  // check for direct io?
  if(caps_req & NIC_CAP_DIRECT_IO) {
    uart_send_pstring(PSTR(" DIO"));
    is_direct_io = 1;
  } else {
    is_direct_io = 0;
  }

  if(is_attached) {
    uart_send_pstring(PSTR(": already attached!"));
    uart_send_crlf();
    return NIC_ERROR_ALREADY_ATTACHED;
  }

  // call init
  u08 result = nic_mod_attach(&caps_req, port, mac);
  if(result == NIC_OK) {

    is_attached = 1;
    caps_in_use = caps_req;

    uart_send_pstring(PSTR(": ok, res_caps="));
    uart_send_hex_word(caps_req);

    // show revision
    u08 rev;
    result = nic_mod_ioctl(NIC_IOCTL_GET_HW_VERSION, &rev);
    if(result == NIC_OK) {
      uart_send_pstring(PSTR(" rev="));
      uart_send_hex_byte(rev);
    }

    // show link status
    u08 status;
    result = nic_mod_ioctl(NIC_IOCTL_GET_LINK_STATUS, &status);
    if(result == NIC_OK) {
      uart_send_pstring(PSTR(" link="));
      uart_send_hex_byte(status);
    }

  } else {
    uart_send_pstring(PSTR("ERROR:"));
    uart_send_hex_byte(result);
  }
  uart_send_crlf();
  return result;
}

void nic_detach(void)
{
  if(!is_attached) {
    return;
  }

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("nic_detach"));
  uart_send_crlf();

  nic_mod_detach();

  is_attached = 0;
  caps_in_use = 0;
}

u08 nic_is_attached(void)
{
  return is_attached;
}

void nic_ping(void)
{
  if(is_attached) {
    nic_mod_ping();
  }
}

void nic_status(void)
{
  if(is_attached) {
    nic_mod_status();
  }
}

u16 nic_caps_available(void)
{
  return nic_mod_caps();
}

u16 nic_caps_in_use(void)
{
  return caps_in_use;
}

u08 nic_is_direct(void)
{
  return is_direct_io;
}

u08 nic_rx_num_pending(void)
{
  if(is_attached) {
    return nic_mod_rx_num_pending();
  } else {
    return 0;
  }
}

u08 nic_rx_size(u16 *got_size)
{
  if(is_attached) {
    return nic_mod_rx_size(got_size);
  } else {
    return NIC_ERROR_NOT_ATTACHED;
  }
}

u08 nic_rx_data(u08 *buf, u16 size)
{
  if(is_attached) {
    return nic_mod_rx_data(buf, size);
  } else {
    return NIC_ERROR_NOT_ATTACHED;
  }
}

u08 nic_tx_data(const u08 *buf, u16 size)
{
  if(is_attached) {
    return nic_mod_tx_data(buf, size);
  } else {
    return NIC_ERROR_NOT_ATTACHED;
  }
}

void nic_rx_direct_begin(u16 size)
{
  if(is_attached) {
    nic_mod_rx_direct_begin(size);
  }
}

u08 nic_rx_direct_end(u16 size)
{
  if(is_attached) {
    return nic_mod_rx_direct_end(size);
  } else {
    return NIC_ERROR_NOT_ATTACHED;
  }
}

void nic_tx_direct_begin(u16 size)
{
  if(is_attached) {
    nic_mod_tx_direct_begin(size);
  }
}

u08 nic_tx_direct_end(u16 size)
{
  if(is_attached) {
    return nic_mod_tx_direct_end(size);
  } else {
    return NIC_ERROR_NOT_ATTACHED;
  }
}

u08 nic_ioctl(u08 ioctl, u08 *value)
{
  if(is_attached) {
    return nic_mod_ioctl(ioctl, value);
  } else {
    return NIC_ERROR_NOT_ATTACHED;
  }
}
