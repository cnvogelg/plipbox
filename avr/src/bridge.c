/*
 * bridge.c: bridge packets from plip to eth and back
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

#include "global.h"
   
#include "pkt_buf.h"
#include "pb_proto.h"
#include "uartutil.h"
#include "param.h"
#include "dump.h"
#include "timer.h"
#include "stats.h"
#include "util.h"
#include "bridge.h"
#include "main.h"
#include "cmd.h"
#include "pb_util.h"
#include "pio_util.h"
#include "pio.h"

static u16 pending_pkt_size;

// ----- packet callbacks -----

static u08 fill_pkt(u08 *buf, u16 max_size, u16 *size)
{
  // use pending packet
  *size = pending_pkt_size;

  // consume packet
  pending_pkt_size = 0;

  return PBPROTO_STATUS_OK;  
}

static u08 proc_pkt(const u08 *buf, u16 size)
{
  // send packet via pio
  pio_util_send_packet(size);

  return PBPROTO_STATUS_OK;
}

// ----- function table -----

static pb_proto_funcs_t funcs = {
  .fill_pkt = fill_pkt,
  .proc_pkt = proc_pkt
};

// ---------- loop ----------

u08 bridge_loop(void)
{
  u08 reset = 0;

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[BRIDGE] on\r\n"));

  pb_proto_init(&funcs, pkt_buf, PKT_BUF_SIZE);
  pio_init(param.mac_addr, PIO_INIT_BROAD_CAST);
  stats_reset();
  
  while(run_mode == RUN_MODE_BRIDGE) {
    // handle commands
    reset = !cmd_worker();
    if(reset) {
      break;
    }

    // handle pbproto
    pb_util_handle();

    // incoming packet via PIO?
    if((pending_pkt_size == 0) && pio_has_recv()) {
      u16 size;
      if(pio_util_recv_packet(&size) == PIO_OK) {
        // request receive
        pending_pkt_size = size;
        pb_proto_request_recv();
      }
    }
  }

  stats_dump_all();
  pio_exit();

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[BRIDGE] off\r\n"));

  return reset;
}
