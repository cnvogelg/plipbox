/*
 * pb_proto.h - avr implementation of the plipbox protocol
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

#ifndef PBPROTO_H
#define PBPROTO_H

#include "global.h"

// status return
#define PBPROTO_STATUS_IDLE              0
#define PBPROTO_STATUS_OK                1
#define PBPROTO_STATUS_TIMEOUT           2
#define PBPROTO_STATUS_LOST_SELECT       3
#define PBPROTO_STATUS_INVALID_CMD       4

// protocol stages for error reprots
#define PBPROTO_STAGE_END_SELECT         0x10
#define PBPROTO_STAGE_SIZE_HI            0x20
#define PBPROTO_STAGE_SIZE_LO            0x30
#define PBPROTO_STAGE_DATA               0x40
#define PBPROTO_STAGE_LAST_DATA          0x50

// commands
#define PBPROTO_CMD_SEND       0x11   // amiga wants to send a packet
#define PBPROTO_CMD_RECV       0x22   // amiga wants to receive a packet

// line status
#define PBPROTO_LINE_OFF       0x0
#define PBPROTO_LINE_DISABLED  0x7
#define PBPROTO_LINE_OK        0x1

// callbacks
typedef void (*pb_proto_begin_func)(u16 *size);
typedef void (*pb_proto_data_func)(u08 *data);
typedef void (*pb_proto_end_func)(u16 size);

typedef struct {
  pb_proto_begin_func    send_begin; // size is given by amiga
  pb_proto_data_func     send_data;
  pb_proto_end_func      send_end;
  
  pb_proto_begin_func    recv_begin; // size must be given by plipbox
  pb_proto_data_func     recv_data;
  pb_proto_end_func      recv_end;
} pb_proto_funcs_t;

// ----- Profiling -----

typedef struct {
  u32 can_enter;
  u32 enter;
  u32 data_begin;
  u32 data_end;
  u32 leave;
} pb_proto_timestamps_t;

extern pb_proto_timestamps_t pb_proto_timestamps;

// ----- Parameter -----

extern u16 pb_proto_rx_timeout; // timeout for next byte in 100us

// ----- API -----

extern void pb_proto_init(pb_proto_funcs_t *f);
extern u08  pb_proto_get_line_status(void);
extern u08  pb_proto_handle(u08 *cmd);
extern void pb_proto_request_recv(void);

#endif
