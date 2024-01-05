/*
 * pio_dev.h - generic interface for packet I/O devices
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

#ifndef PIO_DEV_H
#define PIO_DEV_H

#include "arch.h"
#include "types.h"

/* function pointers */
typedef u08  (*pio_dev_init_t)(u08 flags);
typedef void (*pio_dev_exit_t)(void);

typedef void (*pio_dev_set_mac_t)(const u08 mac_addr[6]);
typedef void (*pio_dev_enable_rx_t)(void);
typedef void (*pio_dev_disable_rx_t)(void);

typedef u08  (*pio_dev_send_t)(const u08 *buf, u16 size);
typedef u08  (*pio_dev_recv_size_t)(u16 *got_size);
typedef u08  (*pio_dev_recv_t)(u08 *buf, u16 size);
typedef u08  (*pio_dev_has_recv_t)(void);

typedef u08  (*pio_dev_status_t)(u08 status_id, u08 *value);
typedef u08  (*pio_dev_control_t)(u08 control_id, u08 value);

/* device structure */
typedef struct {
  const char         *name;

  pio_dev_init_t      init_f;
  pio_dev_exit_t      exit_f;

  pio_dev_set_mac_t    set_mac_f;
  pio_dev_enable_rx_t  enable_rx_f;
  pio_dev_disable_rx_t disable_rx_f;

  pio_dev_send_t      send_f;
  pio_dev_recv_size_t recv_size_f;
  pio_dev_recv_t      recv_f;
  pio_dev_has_recv_t  has_recv_f;

  pio_dev_status_t    status_f;
  pio_dev_control_t   control_f;
} pio_dev_t;

typedef const pio_dev_t *pio_dev_ptr_t;

/* access device data from PROGMEM pio_dev_t */

static inline rom_pchar pio_dev_name(pio_dev_ptr_t pd)
{ 
  return (rom_pchar)read_rom_rom_ptr(&pd->name);
}

static inline u08 pio_dev_init(pio_dev_ptr_t pd, u08 flags)
{
  pio_dev_init_t init_f = (pio_dev_init_t)read_rom_rom_ptr(&pd->init_f);
  return init_f(flags);
}

static inline void pio_dev_exit(pio_dev_ptr_t pd)
{
  pio_dev_exit_t exit_f = (pio_dev_exit_t)read_rom_rom_ptr(&pd->exit_f);
  exit_f();
}

static inline void pio_dev_set_mac(pio_dev_ptr_t pd, const u08 mac[6])
{
  pio_dev_set_mac_t set_mac_f = (pio_dev_set_mac_t)read_rom_rom_ptr(&pd->set_mac_f);
  set_mac_f(mac);
}

static inline void pio_dev_enable_rx(pio_dev_ptr_t pd)
{
  pio_dev_enable_rx_t enable_rx_f = (pio_dev_enable_rx_t)read_rom_rom_ptr(&pd->enable_rx_f);
  enable_rx_f();
}

static inline void pio_dev_disable_rx(pio_dev_ptr_t pd)
{
  pio_dev_disable_rx_t disable_rx_f = (pio_dev_disable_rx_t)read_rom_rom_ptr(&pd->disable_rx_f);
  disable_rx_f();
}

static inline u08 pio_dev_send(pio_dev_ptr_t pd, const u08 *buf, u16 size)
{
  pio_dev_send_t send_f = (pio_dev_send_t)read_rom_rom_ptr(&pd->send_f);
  return send_f(buf, size);
}

static inline u08 pio_dev_recv_size(pio_dev_ptr_t pd, u16 *got_size)
{
  pio_dev_recv_size_t recv_size_f = (pio_dev_recv_size_t)read_rom_rom_ptr(&pd->recv_size_f);
  return recv_size_f(got_size);
}

static inline u08 pio_dev_recv(pio_dev_ptr_t pd, u08 *buf, u16 size)
{
  pio_dev_recv_t recv_f = (pio_dev_recv_t)read_rom_rom_ptr(&pd->recv_f);
  return recv_f(buf, size);
}

static inline u08 pio_dev_has_recv(pio_dev_ptr_t pd)
{
  pio_dev_has_recv_t has_recv_f = (pio_dev_has_recv_t)read_rom_rom_ptr(&pd->has_recv_f);
  return has_recv_f();
}

static inline u08 pio_dev_status(pio_dev_ptr_t pd, u08 status_id, u08 *value)
{
  pio_dev_status_t status_f = (pio_dev_status_t)read_rom_rom_ptr(&pd->status_f);
  return status_f(status_id, value);
}

static inline u08 pio_dev_control(pio_dev_ptr_t pd, u08 control_id, u08 value)
{
  pio_dev_control_t control_f = (pio_dev_control_t)read_rom_rom_ptr(&pd->control_f);
  return control_f(control_id, value);
}

#endif
