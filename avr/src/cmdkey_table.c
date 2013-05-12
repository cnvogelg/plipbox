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

#include "uartutil.h"
#include "timer.h"
   
#include "net/net.h"
#include "net/udp.h"
#include "net/eth.h"

#include "pkt_buf.h"
#include "enc28j60.h"

#include "stats.h"

COMMAND_KEY(cmd_dump_stats)
{
  stats_dump();
}

COMMAND_KEY(cmd_reset_stats)
{
  stats_reset();
}

cmdkey_table_t cmdkey_table[] = {
  { 'd', cmd_dump_stats },
  { 'r', cmd_reset_stats },
  { 0,0 }
};
