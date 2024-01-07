/*
 * mode_mod.h - operation mode modulde of plipbox
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

#ifndef MODE_MOD_H
#define MODE_MOD_H

#include "arch.h"
#include "types.h"

/* function pointers */
typedef u08  (*mode_mod_attach_t)(void);
typedef void (*mode_mod_detach_t)(void);

typedef u08  (*mode_mod_rx_poll_t)(void);

typedef u08* (*mode_mod_tx_begin_t)(u16 size);
typedef u08  (*mode_mod_tx_end_t)(u16 size);

typedef u16  (*mode_mod_rx_size_t)(void);
typedef u08* (*mode_mod_rx_begin_t)(u16 size);
typedef u08  (*mode_mod_rx_end_t)(u16 size);

/* device structure */
typedef struct {
  const char         *name;

  mode_mod_attach_t       attach;
  mode_mod_detach_t       detach;

  mode_mod_rx_poll_t      rx_poll;

  mode_mod_tx_begin_t     tx_begin;
  mode_mod_tx_end_t       tx_end;

  mode_mod_rx_size_t      rx_size;
  mode_mod_rx_begin_t     rx_begin;
  mode_mod_rx_end_t       rx_end;
} mode_mod_t;

typedef const mode_mod_t *mode_mod_ptr_t;

extern mode_mod_ptr_t   mode_mod_ptr;

void mode_mod_init(void);
u08  mode_mod_get_num_modules(void);
void mode_mod_set_current(u08 index);
u08  mode_mod_get_current(void);

/* access device data from PROGMEM mode_mod_t */

static inline const rom_pchar mode_mod_name(void)
{
  mode_mod_ptr_t pd = mode_mod_ptr;
  return (rom_pchar)read_rom_rom_ptr(&pd->name);
}

static inline u08 mode_mod_attach(void)
{
  mode_mod_ptr_t pd = mode_mod_ptr;
  mode_mod_attach_t attach = (mode_mod_attach_t)read_rom_rom_ptr(&pd->attach);
  return attach();
}

static inline void mode_mod_detach(void)
{
  mode_mod_ptr_t pd = mode_mod_ptr;
  mode_mod_detach_t detach = (mode_mod_detach_t)read_rom_rom_ptr(&pd->detach);
  detach();
}

static inline u08 mode_mod_rx_poll(void)
{
  mode_mod_ptr_t pd = mode_mod_ptr;
  mode_mod_rx_poll_t rx_poll = (mode_mod_rx_poll_t)read_rom_rom_ptr(&pd->rx_poll);
  return rx_poll();
}

static inline u08 *mode_mod_tx_begin(u16 size)
{
  mode_mod_ptr_t pd = mode_mod_ptr;
  mode_mod_tx_begin_t tx_begin = (mode_mod_tx_begin_t)read_rom_rom_ptr(&pd->tx_begin);
  return tx_begin(size);
}

static inline u08 mode_mod_tx_end(u16 size)
{
  mode_mod_ptr_t pd = mode_mod_ptr;
  mode_mod_tx_end_t tx_end = (mode_mod_tx_end_t)read_rom_rom_ptr(&pd->tx_end);
  return tx_end(size);
}

static inline u16 mode_mod_rx_size(void)
{
  mode_mod_ptr_t pd = mode_mod_ptr;
  mode_mod_rx_size_t rx_size = (mode_mod_rx_size_t)read_rom_rom_ptr(&pd->rx_size);
  return rx_size();
}

static inline u08 *mode_mod_rx_begin(u16 size)
{
  mode_mod_ptr_t pd = mode_mod_ptr;
  mode_mod_rx_begin_t rx_begin = (mode_mod_rx_begin_t)read_rom_rom_ptr(&pd->rx_begin);
  return rx_begin(size);
}

static inline u08 mode_mod_rx_end(u16 size)
{
  mode_mod_ptr_t pd = mode_mod_ptr;
  mode_mod_rx_end_t rx_end = (mode_mod_rx_end_t)read_rom_rom_ptr(&pd->rx_end);
  return rx_end(size);
}

#endif
