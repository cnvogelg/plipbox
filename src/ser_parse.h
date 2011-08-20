/*
 * ser_parse.h - handle serial line live and command mode
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

#ifndef SER_PARSE_H
#define SER_PARSE_H

#include "global.h"

#define SER_PARSE_CMD_OK      0
#define SER_PARSE_CMD_EXIT    1
#define SER_PARSE_CMD_FAIL    2
#define SER_PARSE_CMD_UNKNOWN 3
#define SER_PARSE_CMD_SILENT  4

typedef void (*ser_parse_data_func_t)(u08 data);
typedef u08 (*ser_parse_cmd_func_t)(u08 len, const char *cmd);
typedef void (*ser_parse_func_t)(void);

// init serial parser and set slip data and end functions
extern void ser_parse_set_data_func(ser_parse_data_func_t data_func_t);
extern void ser_parse_set_cmd_func(ser_parse_cmd_func_t cmd_func_t);
extern void ser_parse_set_cmd_enter_leave_func(ser_parse_func_t enter_func, ser_parse_func_t leave_func);

// return true: state change
extern u08 ser_parse_worker(void);

#endif