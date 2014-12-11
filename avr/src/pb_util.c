/*
 * pb_util.c: plipbox protocol high level helpers
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

#include "pb_util.h"

#include "pb_proto.h"
#include "timer.h"
#include "stats.h"
#include "dump.h"
#include "main.h"

u08 pb_util_handle(void)
{
  // call protocol handler (low level transmit)
  u08 status = pb_proto_handle();
  // nothing done... return
  if(status == PBPROTO_STATUS_IDLE) {
    return PBPROTO_STATUS_IDLE; // inactive
  }

  const pb_proto_stat_t *ps = &pb_proto_stat;

  // ok!
  if(status == PBPROTO_STATUS_OK) {
    // account data
    stats_update_ok(ps->stats_id, ps->size, ps->rate);
    // dump result?
    if(global_verbose) {
      // in interactive mode show result
      dump_pb_cmd(ps);
    }
  }
  // pb proto failed with an error
  else {
    // dump error
    dump_pb_cmd(ps);
    // account data
    stats_get(ps->stats_id)->err++;
  }
  return status;
}
