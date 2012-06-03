/*
 * plip.h - avr implementation of magPLIP protocol
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plip2slip.
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

#ifndef PLIP_H
#define PLIP_H

#include "global.h"

#define PLIP_STATUS_IDLE              0
#define PLIP_STATUS_OK                1
#define PLIP_STATUS_NO_MAGIC          2
#define PLIP_STATUS_INVALID_MAGIC     3
#define PLIP_STATUS_TIMEOUT           4
#define PLIP_STATUS_CALLBACK_FAILED   5
#define PLIP_STATUS_CANT_SEND         6
#define PLIP_STATUS_LOST_SELECT       7

#define PLIP_STATE_MAGIC            0x10
#define PLIP_STATE_CRC_TYPE         0x20
#define PLIP_STATE_SIZE             0x30
#define PLIP_STATE_CRC              0x40
#define PLIP_STATE_TYPE             0x50
#define PLIP_STATE_DATA             0x60
#define PLIP_STATE_LAST_DATA        0x70
#define PLIP_STATE_END              0x80
#define PLIP_STATE_START            0x90

#define PLIP_MAGIC        0x42
#define PLIP_NOCRC        0x02
#define PLIP_CRC          0x01

#define PLIP_LINE_OFF       0x0
#define PLIP_LINE_DISABLED  0x7
#define PLIP_LINE_OK        0x1

typedef struct {
  u08 crc_type;
  u16 size;
  u16 crc;
  u32 type;
  u16 real_size; // will be set after tx/rx
} plip_packet_t;

typedef u08 (*plip_packet_func)(plip_packet_t *pkt);
typedef u08 (*plip_data_func)(u08 *data);

// ----- Parameter -----

extern u16 plip_rx_timeout; // timeout for next byte in 100us

// ----- Init -----

extern void plip_recv_init(plip_packet_func rx_begin_func, 
                           plip_data_func rx_fill_func,
                           plip_packet_func rx_end_func);

extern void plip_send_init(plip_data_func tx_fill_func);

// ----- Line Status -----

extern u08 plip_get_line_status(void);

// ----- Rx/Tx -----

extern u08 plip_can_recv(void);
extern u08 plip_recv(plip_packet_t *pkt);
extern u08 plip_send(plip_packet_t *pkt);

#endif
