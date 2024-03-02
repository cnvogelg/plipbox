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
#include "cmd.h"

#include "arch.h"
#include "stats.h"
#include "uartutil.h"
#include "mode.h"
#include "param.h"
#include "nic_test.h"

COMMAND_KEY(cmd_dump_stats)
{
  stats_dump_all();
  return CMD_WORKER_DONE;
}

COMMAND_KEY(cmd_reset_stats)
{
  stats_reset();
  return CMD_WORKER_DONE;
}

COMMAND_KEY(cmd_toggle_verbose)
{
  return CMD_WORKER_DONE;
}

COMMAND_KEY(cmd_print_version)
{
  uart_send_pstring(PSTR(VERSION " " BUILD_DATE "\r\n"));
  return CMD_WORKER_DONE;
}

COMMAND_KEY(cmd_attach)
{
  mode_attach();
  return CMD_WORKER_DONE;
}

COMMAND_KEY(cmd_detach)
{
  mode_detach();
  return CMD_WORKER_DONE;
}

COMMAND_KEY(cmd_dump_param)
{
  param_dump();
  return CMD_WORKER_DONE;
}

COMMAND_KEY(cmd_nic_test_status)
{
  nic_test_status();
  return CMD_WORKER_DONE;
}

COMMAND_KEY(cmd_nic_test_tx)
{
  nic_test_tx();
  return CMD_WORKER_DONE;
}

COMMAND_KEY(cmd_nic_test_rx)
{
  nic_test_rx();
  return CMD_WORKER_DONE;
}

COMMAND_KEY(cmd_nic_test_toggle_duplex)
{
  nic_test_toggle_duplex();
  return CMD_WORKER_DONE;
}

COMMAND_KEY(cmd_exit)
{
  return CMD_WORKER_EXIT;
}

CMDKEY_HELP(cmd_dump_stats, "dump statistics");
CMDKEY_HELP(cmd_reset_stats, "reset statistics");
CMDKEY_HELP(cmd_toggle_verbose, "toggle verbose output");
CMDKEY_HELP(cmd_print_version, "print version");
CMDKEY_HELP(cmd_attach, "attach device");
CMDKEY_HELP(cmd_detach, "detach device");
CMDKEY_HELP(cmd_dump_param, "dump parameters");
CMDKEY_HELP(cmd_nic_test_status, "test NIC status");
CMDKEY_HELP(cmd_nic_test_tx, "test NIC tx");
CMDKEY_HELP(cmd_nic_test_rx, "test NIC rx");
CMDKEY_HELP(cmd_nic_test_toggle_duplex, "toggle NIC duplex");
CMDKEY_HELP(cmd_exit, "exit current loop");

const cmdkey_table_t ROM_ATTR cmdkey_table[] = {
  CMDKEY_ENTRY('s', cmd_dump_stats),
  CMDKEY_ENTRY('S', cmd_reset_stats),
  CMDKEY_ENTRY('v', cmd_toggle_verbose),
  CMDKEY_ENTRY('V', cmd_print_version),
  CMDKEY_ENTRY('a', cmd_attach),
  CMDKEY_ENTRY('d', cmd_detach),
  CMDKEY_ENTRY('p', cmd_dump_param),
  CMDKEY_ENTRY('n', cmd_nic_test_status),
  CMDKEY_ENTRY('t', cmd_nic_test_tx),
  CMDKEY_ENTRY('r', cmd_nic_test_rx),
  CMDKEY_ENTRY('f', cmd_nic_test_toggle_duplex),
  CMDKEY_ENTRY('x', cmd_exit),
  { 0,0 }
};
