/*
 * mode.h - handle operation mode of plipbox
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

#ifndef MODE_H
#define MODE_H

#include "mode_shared.h"

typedef struct {
  u32  tag;
} mode_def_t;

void mode_init(void);

// mode_mod management
void mode_dump_modes(void);
u08  mode_get_num_modes(void);
u08  mode_find_tag(u32 tag);
void mode_get_def(u08 index, mode_def_t *def);

// callbacks from cmds
u08 mode_attach(void);
u08 mode_detach(void);
u08 mode_get_attached_mode(void);
void mode_set_request_mode(u08 mode);
void mode_ping(void);

// packet io
u08 *mode_tx_begin(u16 size);
u16  mode_tx_end(u16 size);

u16  mode_rx_size(void);
u08 *mode_rx_begin(u16 size);
u16  mode_rx_end(u16 size);

// main worker
void mode_work(void);

#endif
