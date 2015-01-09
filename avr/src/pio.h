/*
 * pio.h - generic interface for packet I/O
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

#ifndef PIO_H
#define PIO_H

#include "global.h"

/* result values */
#define PIO_OK            0
#define PIO_NOT_FOUND     1
#define PIO_TOO_LARGE     2
#define PIO_IO_ERR        3

/* init flags */
#define PIO_INIT_FULL_DUPLEX    1
#define PIO_INIT_LOOP_BACK      2
#define PIO_INIT_BROAD_CAST     4
#define PIO_INIT_FLOW_CONTROL   8

/* status flags */
#define PIO_STATUS_VERSION      0
#define PIO_STATUS_LINK_UP      1 

/* control ids */
#define PIO_CONTROL_FLOW        0

/* --- API --- */

extern u08 pio_set_device(u08 id);
extern u08 pio_init(const u08 mac[6],u08 flags);
extern void pio_exit(void);

extern u08 pio_send(const u08 *buf, u16 size);
extern u08 pio_recv(u08 *buf, u16 max_size, u16 *got_size);
extern u08 pio_has_recv(void);
extern u08 pio_status(u08 status_id, u08 *value);
extern u08 pio_control(u08 control_id, u08 value);

#endif
