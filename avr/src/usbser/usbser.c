/*
 * usbser.c - transfer via USB serial
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
#include "usbser.h"
#include "usb_serial.h"
#include "uartutil.h"
#include "timer.h"

typedef struct {
  u08 magic;
  u08 version;
  u16 size;
} header_t;

#define MAGIC_BYTE  0x42
#define VERSION     0x06

static u08 usbser_init(const u08 macaddr[6], u08 flags)
{
  usb_init();
  return PIO_OK;
}

static void usbser_exit(void)
{
  usb_exit();
}

static u08 usbser_control(u08 control_id, u08 value)
{
  switch(control_id) {
    case PIO_CONTROL_FLOW:
      return PIO_OK;
    default:
      return PIO_NOT_FOUND;
  }
}

static u08 usbser_status(u08 status_id, u08 *value)
{
  switch(status_id) {
    case PIO_STATUS_VERSION:
      *value = 0;
      return PIO_OK;
    case PIO_STATUS_LINK_UP:
      *value = 0;
      return PIO_OK;
    default:
      *value = 0;
      return PIO_NOT_FOUND;
  }
}

//#define DEBUG_USBSER

static u08 usbser_send(const u08 *data, u16 size)
{
  header_t h = { MAGIC_BYTE, VERSION, size };

  // send header
#ifdef DEBUG_USBSER
  uart_send_pstring(PSTR("TX:"));
  timer_hw_reset();
#endif

  u08 error = usb_serial_write((const u08 *)&h, sizeof(h));
  if(error) {
#ifdef DEBUG_USBSER
    uart_send_pstring(PSTR("!\r\n"));
#endif
    return PIO_IO_ERR;
  }

  // send data
  error = usb_serial_write(data, size);
  if(error) {
#ifdef DEBUG_USBSER
    uart_send_pstring(PSTR("!\r\n"));
#endif
    return PIO_IO_ERR;
  }

#ifdef DEBUG_USBSER
  u16 delta = timer_hw_get();
  u16 rate = timer_hw_calc_rate_kbs(size, delta);
  uart_send_hex_word(delta);
  uart_send_spc();
  uart_send_hex_word(size);
  uart_send_spc();
  uart_send_rate_kbs(rate);
  uart_send_crlf();
#endif
  return PIO_OK;
}

static u08 usbser_has_recv(void)
{
  return usb_serial_available() > 0;
}

static u08 usbser_recv(u08 *data, u16 max_size, u16 *got_size)
{
  header_t h;

  // recv header
#ifdef DEBUG_USBSER
  uart_send_pstring(PSTR("RX:"));
  timer_hw_reset();
#endif
  u08 error = usb_serial_read((u08 *)&h, sizeof(h));
  if(error) {
#ifdef DEBUG_USBSER
    uart_send_pstring(PSTR("!\r\n"));
#endif
    return PIO_IO_ERR;
  }

  // check header
  if(h.magic != MAGIC_BYTE) {
#ifdef DEBUG_USBSER
    uart_send_hex_byte(h.magic);
    uart_send_pstring(PSTR(" MAGIC?\r\n"));
#endif
    return PIO_IO_ERR;
  }
  if(h.version != VERSION) {
#ifdef DEBUG_USBSER
    uart_send_pstring(PSTR("VERSION?\r\n"));
#endif
    return PIO_IO_ERR;
  }
  if(h.size > max_size) {
#ifdef DEBUG_USBSER
    uart_send_pstring(PSTR("MAX?\r\n"));
#endif
    return PIO_IO_ERR;
  }

  // read data
  error = usb_serial_read(data, h.size);
  if(error) {
#ifdef DEBUG_USBSER
    uart_send_pstring(PSTR("!\r\n"));
#endif
    return PIO_IO_ERR;
  }

#ifdef DEBUG_USBSER
  u16 delta = timer_hw_get();
  u16 rate = timer_hw_calc_rate_kbs(h.size, delta);
  uart_send_hex_word(delta);
  uart_send_spc();
  uart_send_hex_word(h.size);
  uart_send_spc();
  uart_send_rate_kbs(rate);
  uart_send_crlf();
#endif
  *got_size = h.size;
  return PIO_OK;
}

// ----- pio_dev -----
static const char PROGMEM dev_name[] = "usbser";
const pio_dev_t PROGMEM pio_dev_usbser = {
  .name = dev_name,
  .init_f = usbser_init,
  .exit_f = usbser_exit,
  .send_f = usbser_send,
  .recv_f = usbser_recv,
  .has_recv_f = usbser_has_recv,
  .status_f = usbser_status,
  .control_f = usbser_control
};
