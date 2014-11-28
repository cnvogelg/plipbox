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

#include <avr/pgmspace.h>
#include "global.h"

/* function pointers */
typedef u08  (*pio_dev_init_t)(const u08 mac[6],u08 flags);
typedef void (*pio_dev_exit_t)(void);
typedef u08  (*pio_dev_send_t)(const u08 *buf, u16 size);
typedef u08  (*pio_dev_recv_t)(u08 *buf, u16 max_size, u16 *got_size);
typedef u08  (*pio_dev_has_recv_t)(void);
typedef u08  (*pio_dev_status_t)(u08 status_id, u08 *value);
typedef u08  (*pio_dev_control_t)(u08 control_id, u08 value);

/* device structure */
typedef struct {
  const char         *name;
  pio_dev_init_t      init_f;
  pio_dev_exit_t      exit_f;
  pio_dev_send_t      send_f;
  pio_dev_recv_t      recv_f;
  pio_dev_has_recv_t  has_recv_f;
  pio_dev_status_t    status_f;
  pio_dev_control_t   control_f;
} pio_dev_t;

typedef const pio_dev_t *pio_dev_ptr_t;

/* access device data from PROGMEM pio_dev_t */

inline const PGM_P pio_dev_name(pio_dev_ptr_t pd)
{ 
  return (PGM_P)pgm_read_word(&pd->name);
}

inline u08 pio_dev_init(pio_dev_ptr_t pd, const u08 mac[6], u08 flags)
{
  pio_dev_init_t init_f = (pio_dev_init_t)pgm_read_word(&pd->init_f);
  return init_f(mac, flags);
}

inline void pio_dev_exit(pio_dev_ptr_t pd)
{
  pio_dev_exit_t exit_f = (pio_dev_exit_t)pgm_read_word(&pd->exit_f);
  exit_f();
}

inline u08 pio_dev_send(pio_dev_ptr_t pd, const u08 *buf, u16 size)
{
  pio_dev_send_t send_f = (pio_dev_send_t)pgm_read_word(&pd->send_f);
  return send_f(buf, size);
}

inline u08 pio_dev_recv(pio_dev_ptr_t pd, u08 *buf, u16 max_size, u16 *got_size)
{
  pio_dev_recv_t recv_f = (pio_dev_recv_t)pgm_read_word(&pd->recv_f);
  return recv_f(buf, max_size, got_size);
}

inline u08 pio_dev_has_recv(pio_dev_ptr_t pd)
{
  pio_dev_has_recv_t has_recv_f = (pio_dev_has_recv_t)pgm_read_word(&pd->has_recv_f);
  return has_recv_f();
}

inline u08 pio_dev_status(pio_dev_ptr_t pd, u08 status_id, u08 *value)
{
  pio_dev_status_t status_f = (pio_dev_status_t)pgm_read_word(&pd->status_f);
  return status_f(status_id, value);
}

inline u08 pio_dev_control(pio_dev_ptr_t pd, u08 control_id, u08 value)
{
  pio_dev_control_t control_f = (pio_dev_control_t)pgm_read_word(&pd->control_f);
  return control_f(control_id, value);
}

#endif
