/*
 * nic_mod.h - generic interface for NIC modules
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

#ifndef NIC_DEV_H
#define NIC_DEV_H

#include "arch.h"
#include "types.h"

/* function pointers */
typedef u08  (*nic_mod_attach_t)(u08 flags, mac_t mac);
typedef void (*nic_mod_detach_t)(void);

typedef void (*nic_mod_enable_t)(void);
typedef void (*nic_mod_disable_t)(void);

typedef u08  (*nic_mod_rx_num_pending_t)(void);
typedef u08  (*nic_mod_rx_size_t)(u16 *got_size);

typedef u08  (*nic_mod_rx_data_t)(u08 *buf, u16 size);
typedef u08  (*nic_mod_tx_data_t)(const u08 *buf, u16 size);

typedef u08  (*nic_mod_rx_direct_begin_t)(void);
typedef u08  (*nic_mod_rx_direct_end_t)(void);
typedef u08  (*nic_mod_tx_direct_begin_t)(u16 size);
typedef u08  (*nic_mod_tx_direct_end_t)(void);

typedef u08  (*nic_mod_ioctl_t)(u08 ioctl, u08 *value);

/* device structure */
typedef struct {
  const char         *name;
  u16                 capabilities;

  nic_mod_attach_t    attach_f;
  nic_mod_detach_t    detach_f;

  nic_mod_enable_t    enable_f;
  nic_mod_disable_t   disable_f;

  nic_mod_rx_num_pending_t  rx_num_pending_f;
  nic_mod_rx_size_t   rx_size_f;

  nic_mod_rx_data_t   rx_data_f;
  nic_mod_tx_data_t   tx_data_f;

  nic_mod_rx_direct_begin_t  rx_direct_begin_f;
  nic_mod_rx_direct_end_t    rx_direct_end_f;
  nic_mod_tx_direct_begin_t  tx_direct_begin_f;
  nic_mod_tx_direct_end_t    tx_direct_end_f;

  nic_mod_ioctl_t     ioctl_f;
} nic_mod_t;

typedef const nic_mod_t *nic_mod_ptr_t;

extern nic_mod_ptr_t   nic_mod_ptr;

extern void nic_mod_init(void);
extern u08  nic_mod_get_num_modules(void);
extern void nic_mod_set_current(u08 index);
extern u08  nic_mod_get_current(void);

/* access device data from PROGMEM nic_mod_t */

static inline rom_pchar nic_mod_name(void)
{
  nic_mod_ptr_t pd = nic_mod_ptr;
  return (rom_pchar)read_rom_rom_ptr(&pd->name);
}

static inline u08 nic_mod_attach(u08 flags, mac_t mac)
{
  nic_mod_ptr_t pd = nic_mod_ptr;
  nic_mod_attach_t attach_f = (nic_mod_attach_t)read_rom_rom_ptr(&pd->attach_f);
  return attach_f(flags, mac);
}

static inline void nic_mod_detach(void)
{
  nic_mod_ptr_t pd = nic_mod_ptr;
  nic_mod_detach_t detach_f = (nic_mod_detach_t)read_rom_rom_ptr(&pd->detach_f);
  detach_f();
}

static inline void nic_mod_enable(void)
{
  nic_mod_ptr_t pd = nic_mod_ptr;
  nic_mod_enable_t enable_f = (nic_mod_enable_t)read_rom_rom_ptr(&pd->enable_f);
  enable_f();
}

static inline void nic_mod_disable(void)
{
  nic_mod_ptr_t pd = nic_mod_ptr;
  nic_mod_disable_t disable_f = (nic_mod_disable_t)read_rom_rom_ptr(&pd->disable_f);
  disable_f();
}

static inline u08 nic_mod_rx_num_pending(void)
{
  nic_mod_ptr_t pd = nic_mod_ptr;
  nic_mod_rx_num_pending_t rx_num_pending_f = (nic_mod_rx_num_pending_t)read_rom_rom_ptr(&pd->rx_num_pending_f);
  return rx_num_pending_f();
}

static inline u08 nic_mod_rx_size(u16 *got_size)
{
  nic_mod_ptr_t pd = nic_mod_ptr;
  nic_mod_rx_size_t rx_size_f = (nic_mod_rx_size_t)read_rom_rom_ptr(&pd->rx_size_f);
  return rx_size_f(got_size);
}

static inline u08 nic_mod_rx_data(u08 *buf, u16 size)
{
  nic_mod_ptr_t pd = nic_mod_ptr;
  nic_mod_rx_data_t rx_data_f = (nic_mod_rx_data_t)read_rom_rom_ptr(&pd->rx_data_f);
  return rx_data_f(buf, size);
}

static inline u08 nic_mod_tx_data(const u08 *buf, u16 size)
{
  nic_mod_ptr_t pd = nic_mod_ptr;
  nic_mod_tx_data_t tx_data_f = (nic_mod_tx_data_t)read_rom_rom_ptr(&pd->tx_data_f);
  return tx_data_f(buf, size);
}

static inline u08 nic_mod_rx_direct_begin(void)
{
  nic_mod_ptr_t pd = nic_mod_ptr;
  nic_mod_rx_direct_begin_t rx_direct_begin_f = (nic_mod_rx_direct_begin_t)read_rom_rom_ptr(&pd->rx_direct_begin_f);
  return rx_direct_begin_f();
}

static inline u08 nic_mod_rx_direct_end(void)
{
  nic_mod_ptr_t pd = nic_mod_ptr;
  nic_mod_rx_direct_end_t rx_direct_end_f = (nic_mod_rx_direct_end_t)read_rom_rom_ptr(&pd->rx_direct_end_f);
  return rx_direct_end_f();
}

static inline u08 nic_mod_tx_direct_begin(u16 size)
{
  nic_mod_ptr_t pd = nic_mod_ptr;
  nic_mod_tx_direct_begin_t tx_direct_begin_f = (nic_mod_tx_direct_begin_t)read_rom_rom_ptr(&pd->tx_direct_begin_f);
  return tx_direct_begin_f(size);
}

static inline u08 nic_mod_tx_direct_end(void)
{
  nic_mod_ptr_t pd = nic_mod_ptr;
  nic_mod_tx_direct_end_t tx_direct_end_f = (nic_mod_tx_direct_end_t)read_rom_rom_ptr(&pd->tx_direct_end_f);
  return tx_direct_end_f();
}

static inline u08 nic_mod_ioctl(u08 ioctl, u08 *value)
{
  nic_mod_ptr_t pd = nic_mod_ptr;
  nic_mod_ioctl_t ioctl_f = (nic_mod_ioctl_t)read_rom_rom_ptr(&pd->ioctl_f);
  return ioctl_f(ioctl, value);
}

#endif
