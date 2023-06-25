/*
 * cmdkey_table.c - command key table
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

#include "cmdkey_table.h"

#include "stats.h"
#include "uartutil.h"

COMMAND_KEY(cmd_dump_stats)
{
  stats_dump_all();
}

COMMAND_KEY(cmd_reset_stats)
{
  stats_reset();
}

COMMAND_KEY(cmd_toggle_verbose)
{
  global_verbose = !global_verbose;
  uart_send_pstring(PSTR("VERBOSE: "));
  uart_send_pstring(global_verbose ? PSTR("ON\r\n") : PSTR("OFF\r\n"));
}

CMDKEY_HELP(cmd_dump_stats, "dump statistics");
CMDKEY_HELP(cmd_reset_stats, "reset statistics");
CMDKEY_HELP(cmd_toggle_verbose, "toggle verbose output");

const cmdkey_table_t PROGMEM cmdkey_table[] = {
  CMDKEY_ENTRY('s', cmd_dump_stats),
  CMDKEY_ENTRY('S', cmd_reset_stats),
  CMDKEY_ENTRY('v', cmd_toggle_verbose),
  { 0,0 }
};
