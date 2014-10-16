/*
 * pb_io.h: handle incoming plipbox packets
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

#ifndef PLIP_RX_H

#include "global.h"

#define PB_IO_IDLE		0
#define PB_IO_OK        1
#define PB_IO_ERROR     2

#define PB_IO_MAGIC_ONLINE 0xffff

extern void pb_io_init(void);
extern u08 pb_io_worker(u08 plip_state, u08 eth_online);
extern void pb_io_send_magic(u16 type, u08 extra_size);

// SANA info reported by driver
extern u08 sana_online;
extern u08 sana_mac[6];
extern u08 sana_version[2];

#endif
