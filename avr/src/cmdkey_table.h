/*
 * cmdkey_table.h - command key table
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

#ifndef CMDKEY_TABLE_H
#define CMDKEY_TABLE_H

#include <avr/pgmspace.h>
#include "global.h"

#define COMMAND_KEY(x) static void x (void)
#define CMDKEY_HELP(x,y) static const char x ## _help[] PROGMEM = y 
#define CMDKEY_ENTRY(x,y) { x, y, y ## _help }

typedef void (*cmdkey_func_t)(void);

struct cmdkey_table_s {
  u08     key;
  cmdkey_func_t func;
  const char * help;
};
typedef struct cmdkey_table_s cmdkey_table_t;

extern const cmdkey_table_t PROGMEM cmdkey_table[];

#endif
