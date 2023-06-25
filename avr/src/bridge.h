/*
 * bridge.h: bridge packets from plip to eth and back
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

#ifndef BRIDGE_H
#define BRIDGE_H

#include "global.h"

// bridge modes
#define BRIDGE_MODE_FORWARD           0 // normal forwarding via pkt_buf
#define BRIDGE_MODE_LOOPBACK_BUF      1 // copy to pkt_buf and back
#define BRIDGE_MODE_LOOPBACK_DEV_BUF  2 // copy to pkt_buf and then to dev

// bridge transfer
#define BRIDGE_TRANSFER_BUF           0
#define BRIDGE_TRANSFER_SPI           1

extern void bridge_init(u08 pio_ok);
extern void bridge_handle(void);

extern u08 bridge_get_mode(void);
extern void bridge_set_mode(u08 mode);
extern u08 bridge_get_transfer(void);
extern void bridge_set_transfer(u08 mode);

#endif
