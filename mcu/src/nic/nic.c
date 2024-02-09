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

void nic_init(void)
{
  nic_mod_init();
}

void nic_set_device(u08 device)
{
  nic_mod_set_current(device);
}

u08 nic_attach_params(void)
{
  u08 nic = param_get_nic();
  u08 nic_flags = param_get_nic_flags();
  mac_t mac;
  param_get_cur_mac(mac);

  nic_set_device(nic);
  u08 res = nic_attach(nic_flags, mac);

  return res;
}

u08 nic_attach(u08 flags, mac_t mac)
{
  // show hello
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("nic_attach: #"));
  // dev no
  u08 device = nic_mod_get_current();
  uart_send_hex_byte(device);
  uart_send_spc();
  // dev name
  rom_pchar name = nic_mod_name();
  uart_send_pstring(name);
  uart_send_spc();
  // flags
  uart_send_hex_byte(flags);
  uart_send_spc();
  // mac
  uart_send_hex_mac(mac);

  // call init
  u08 result = nic_mod_attach(flags, mac);
  if(result == NIC_OK) {
    uart_send_pstring(PSTR(": ok!"));

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
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("nic_detach"));
  uart_send_crlf();

  nic_mod_detach();

}

u16 nic_capabilites(void)
{
  // TODO
  return 0;
}

u08 nic_is_direct(void)
{
  // TODO
  return 0;
}

void nic_enable(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("nic_enable"));
  uart_send_crlf();

  nic_mod_enable();
}

void nic_disable(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("nic_disable"));
  uart_send_crlf();

  nic_mod_disable();
}

u08 nic_rx_num_pending(void)
{
  return nic_mod_rx_num_pending();
}

u16 nic_rx_size()
{
  return nic_mod_rx_size();
}

u08 nic_rx_data(u08 *buf, u16 size)
{
  return nic_mod_rx_data(buf, size);
}

u08 nic_tx_data(const u08 *buf, u16 size)
{
  return nic_mod_tx_data(buf, size);
}

void nic_rx_direct_begin(u16 size)
{
  nic_mod_rx_direct_begin(size);
}

u08 nic_rx_direct_end(u16 size)
{
  return nic_mod_rx_direct_end(size);
}

void nic_tx_direct_begin(u16 size)
{
  nic_mod_tx_direct_begin(size);
}

u08 nic_tx_direct_end(u16 size)
{
  return nic_mod_tx_direct_end(size);
}

u08 nic_ioctl(u08 ioctl, u08 *value)
{
  return nic_mod_ioctl(ioctl, value);
}
