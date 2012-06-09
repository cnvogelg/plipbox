/*
 * cmd_table.h - command table
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

#ifndef CMD_TABLE_H
#define CMD_TABLE_H

#include <avr/pgmspace.h>
#include "global.h"

#define CMD_OK            0x00
#define CMD_QUIT          0x01

#define CMD_MASK          0xf0
#define CMD_MASK_OK       0x00
#define CMD_MASK_SYNTAX   0x10
#define CMD_MASK_ERROR    0x20

#define COMMAND(x) static u08 x (u08 argc, const u08 **argv)
#define CMD_NAME(x) (const PROGMEM char *)x

struct cmd_table_s {
  PGM_P   name;
  u08     (*func)(u08 argc, const u08 **argv);
};
typedef struct cmd_table_s cmd_table_t;

extern cmd_table_t cmd_table[];

#endif
