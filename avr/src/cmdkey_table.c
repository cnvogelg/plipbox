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
#include "pb_test.h"
#include "main.h"

COMMAND_KEY(cmd_dump_stats)
{
  stats_dump_all();
}

COMMAND_KEY(cmd_reset_stats)
{
  stats_reset();
}

COMMAND_KEY(cmd_enter_pb_test_mode)
{
  run_mode = RUN_MODE_PB_TEST;
}

COMMAND_KEY(cmd_enter_pio_test_mode)
{
  run_mode = RUN_MODE_PIO_TEST;
}

COMMAND_KEY(cmd_enter_bridge_mode)
{
  run_mode = RUN_MODE_BRIDGE;
}

COMMAND_KEY(cmd_enter_bridge_test_mode)
{
  run_mode = RUN_MODE_BRIDGE_TEST;
}

COMMAND_KEY(cmd_send_test_packet)
{
  pb_test_send_packet(0);
}

COMMAND_KEY(cmd_send_test_packet_silent)
{
  pb_test_send_packet(1);
}

COMMAND_KEY(cmd_toggle_auto_mode)
{
  pb_test_toggle_auto();
}

COMMAND_KEY(cmd_toggle_verbose)
{
  global_verbose = !global_verbose;
}

const cmdkey_table_t PROGMEM cmdkey_table[] = {
  { 's', cmd_dump_stats },
  { 'S', cmd_reset_stats },
  { '4', cmd_enter_pb_test_mode },
  { '3', cmd_enter_pio_test_mode },
  { '2', cmd_enter_bridge_test_mode },
  { '1', cmd_enter_bridge_mode },
  { 'p', cmd_send_test_packet },
  { 'P', cmd_send_test_packet_silent },
  { 'a', cmd_toggle_auto_mode },
  { 'v', cmd_toggle_verbose },
  { 0,0 }
};
