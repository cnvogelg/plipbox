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
#include "log.h"
#include "pb_test.h"
#include "pb_io.h"

COMMAND_KEY(cmd_dump_stats)
{
  stats_dump();
}

COMMAND_KEY(cmd_reset_stats)
{
  stats_reset();
}

COMMAND_KEY(cmd_dump_log)
{
  log_dump();
}

COMMAND_KEY(cmd_reset_log)
{
  log_reset();
}

COMMAND_KEY(cmd_toggle_test_mode)
{
  pb_test_toggle_mode();
}

COMMAND_KEY(cmd_send_test_packet)
{
  pb_test_send_packet();
}

COMMAND_KEY(cmd_toggle_auto_mode)
{
  pb_test_toggle_auto();
}

COMMAND_KEY(cmd_send_magic)
{
  pb_io_send_magic(0xffff, 0);
}

cmdkey_table_t cmdkey_table[] = {
  { 's', cmd_dump_stats },
  { 'S', cmd_reset_stats },
  { 'l', cmd_dump_log },
  { 'L', cmd_reset_log },
  { 't', cmd_toggle_test_mode },
  { 'p', cmd_send_test_packet },
  { 'a', cmd_toggle_auto_mode },
  { 'm', cmd_send_magic },
  { 0,0 }
};
