/*
 * pkt_buf.h - manage the packet buffer
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

#ifndef PKT_BUF_H
#define PKT_BUF_H

#include "global.h"
#include "plip.h"

#define PKT_BUF_SIZE    128

/* plip rx side / eth tx */
extern u08 rx_pkt_buf[PKT_BUF_SIZE];
extern plip_packet_t rx_pkt;

/* plip tx side / eth rx */
extern u08 tx_pkt_buf[PKT_BUF_SIZE];
extern plip_packet_t tx_pkt;

#endif
