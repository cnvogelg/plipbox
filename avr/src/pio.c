/*
 * pio.c - generic interface for packet I/O
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

#include "pio.h"
#include "pio_dev.h"
#include "uartutil.h"
#include "net/net.h"

#ifdef DEV_ENC28J60
#include "enc28j60.h"
#endif

// the table of available devices
static const pio_dev_ptr_t PROGMEM devices[] = {
#ifdef DEV_ENC28J60
  &pio_dev_enc28j60,
#endif
};
#define NUM_DEVICES  (sizeof(devices) / sizeof(pio_dev_ptr_t))

// index of active device
static u08 dev_id;
static pio_dev_ptr_t cur_dev;

u08 pio_set_device(u08 id)
{
  if(id < NUM_DEVICES) {
    dev_id = id;
    return 1;
  }
  // invalid device selected
  return 0;
}

u08 pio_init(u08 flags)
{
  // get current device
  cur_dev = (pio_dev_ptr_t)pgm_read_word(devices + dev_id);

  // show hello
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("pio: init: "));
  PGM_P name = pio_dev_name(cur_dev);
  uart_send_pstring(name);

  // call init
  u08 result = pio_dev_init(cur_dev, flags);
  if(result == PIO_OK) {
    uart_send_pstring(PSTR(": ok! flags="));
    uart_send_hex_byte(flags);
    // show revision
    u08 rev;
    result = pio_dev_status(cur_dev, PIO_STATUS_VERSION, &rev);
    if(result == PIO_OK) {
      uart_send_pstring(PSTR(" rev="));
      uart_send_hex_byte(rev);
    }
  } else {
    uart_send_pstring(PSTR("ERROR:"));
    uart_send_hex_byte(result);
  }
  uart_send_crlf();
  return result;
}

void pio_exit(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("pio: exit\r\n"));

  pio_dev_exit(cur_dev);
}

void pio_set_mac(const u08 mac[6])
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("pio: set_mac: "));
  uart_send_hex_mac(mac);
  uart_send_crlf();

  pio_dev_set_mac(cur_dev, mac);
}

void pio_enable_rx(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("pio: enable_rx\r\n"));

  pio_dev_enable_rx(cur_dev);
}

void pio_disable_rx(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("pio: disable_rx\r\n"));

  pio_dev_disable_rx(cur_dev);
}

u08 pio_send(const u08 *buf, u16 size)
{
  return pio_dev_send(cur_dev, buf, size);
}

u08 pio_recv_size(u16 *size)
{
  return pio_dev_recv_size(cur_dev, size);
}

u08 pio_recv(u08 *buf, u16 size)
{
  return pio_dev_recv(cur_dev, buf, size);
}

u08 pio_has_recv(void)
{
  return pio_dev_has_recv(cur_dev);
}

u08 pio_status(u08 status_id, u08 *value)
{
  return pio_dev_status(cur_dev, status_id, value);
}

u08 pio_control(u08 control_id, u08 value)
{
  return pio_dev_control(cur_dev, control_id, value);
}
