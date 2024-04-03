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

#ifdef DEBUG_NIC
#define DEBUG
#endif

#include "debug.h"
#include "nic.h"
#include "nic_mod.h"
#ifdef HAVE_WIFI
#include "nic_wifi.h"
#include "nic_wifi_mod.h"
#endif

#include "uartutil.h"
#include "param.h"

static u08 is_direct_io;
static u08 is_attached;
static u08 cap_link_status;
static u16 caps_in_use;

void nic_init(void)
{
  nic_mod_init();
#ifdef HAVE_WIFI
  nic_wifi_init();
#endif

  is_attached = 0;
}

void nic_set_device(u08 device)
{
  nic_mod_set_current(device);
#ifdef HAVE_WIFI
  nic_wifi_mod_set_current();
#endif
}

u08 nic_get_num_nics(void)
{
  return nic_mod_get_num_nics();
}

u08 nic_find_tag(u32 tag)
{
  u08 num = nic_get_num_nics();
  for(u08 i=0;i<num;i++) {
    u32 mtag = nic_mod_tag_at(i);
    if(mtag == tag) {
      return i;
    }
  }
  return NIC_ID_INVALID;
}

void nic_get_def(u08 index, nic_def_t *def)
{
  nic_mod_def_at(index, def);
}

u08 nic_attach_params(void)
{
  u08 nic = param_get_nic();
  u16 nic_opts = param_get_nic_opts();
  u08 nic_port = param_get_nic_port();
  mac_t mac;
  param_get_cur_mac(mac);

  nic_set_device(nic);
  u08 res = nic_attach(nic_opts, nic_port, mac);

  return res;
}

static void add_opt(u16 opts, u16 opt_flag, u16 *caps, u16 cap_flag)
{
  if((opts & opt_flag) == opt_flag) {
    *caps |= cap_flag;
  }
}

static u16 map_opts(u16 opts, u16 caps)
{
  u16 caps_req = 0;

  add_opt(opts, NIC_OPT_DIRECT_IO, &caps_req, NIC_CAP_DIRECT_IO);
  add_opt(opts, NIC_OPT_LOOP_BACK, &caps_req, NIC_CAP_LOOP_BACK);
  add_opt(opts, NIC_OPT_FULL_DUPLEX, &caps_req, NIC_CAP_FULL_DUPLEX);

  return caps_req;
}

u08 nic_attach(u16 opts, u08 port, mac_t mac)
{
  u08 device = nic_mod_get_current();

  // get caps device has to offer
  u16 caps_available = nic_caps_available();
  // caps we want to use
  caps_in_use = map_opts(opts, caps_available);

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
  uart_send_pstring(PSTR(" opts="));
  uart_send_hex_word(opts);
  uart_send_pstring(PSTR(" caps="));
  uart_send_hex_word(caps_available);
  uart_send('/');
  uart_send_hex_word(caps_in_use);
  // port
  uart_send_pstring(PSTR(" port="));
  uart_send_hex_byte(port);
  // mac
  uart_send_pstring(PSTR(" mac="));
  uart_send_hex_mac(mac);

  if(is_attached) {
    uart_send_pstring(PSTR(": already attached!"));
    uart_send_crlf();
    return NIC_ERROR_ALREADY_ATTACHED;
  }

  // call init
  u08 result = nic_mod_attach(caps_in_use, port, mac);
  if(result == NIC_OK) {

    is_attached = 1;
    uart_send_pstring(PSTR(": ok"));

    // check for direct io?
    if(caps_in_use & NIC_CAP_DIRECT_IO) {
      uart_send_pstring(PSTR(" dio"));
      is_direct_io = 1;
    } else {
      is_direct_io = 0;
    }

    // show revision
    u08 rev;
    u08 res2 = nic_mod_ioctl(NIC_IOCTL_GET_HW_VERSION, &rev);
    if(res2 == NIC_OK) {
      uart_send_pstring(PSTR(" rev="));
      uart_send_hex_byte(rev);
    }

    // if device has link status
    if(caps_available & NIC_CAP_LINK_STATUS) {
      cap_link_status = 1;
      // show link status
      u08 status;
      res2 = nic_mod_ioctl(NIC_IOCTL_GET_LINK_STATUS, &status);
      if(res2 == NIC_OK) {
        uart_send_pstring(PSTR(" link="));
        uart_send_hex_byte(status);
      }
    } else {
      cap_link_status = 0;
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
}

u08 nic_is_attached(void)
{
  return is_attached;
}

u08 nic_has_link_status(void)
{
  return cap_link_status;
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

u08 *nic_rx_begin(u16 size)
{
  if(is_attached) {
    return nic_mod_rx_begin(size);
  } else {
    return NULL;
  }
}

u08 nic_rx_end(u16 size)
{
  if(is_attached) {
    return nic_mod_rx_end(size);
  } else {
    return NIC_ERROR_NOT_ATTACHED;
  }
}

u08 *nic_tx_begin(u16 size)
{
  if(is_attached) {
    return nic_mod_tx_begin(size);
  } else {
    return NULL;
  }
}

u08 nic_tx_end(u16 size)
{
  if(is_attached) {
    return nic_mod_tx_end(size);
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
