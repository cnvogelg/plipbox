/*
 * cmdkey_table.h - command key table
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

#ifndef CMDKEY_TABLE_H
#define CMDKEY_TABLE_H

#include "global.h"

#define COMMAND_KEY(x) static void x (void)

struct cmdkey_table_s {
  u08     key;
  void    (*func)(void);
};
typedef struct cmdkey_table_s cmdkey_table_t;

extern cmdkey_table_t cmdkey_table[];

#endif
