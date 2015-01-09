/*
 * pb_test.c: plipbox test mode
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

#include "pb_test.h"

#include "uartutil.h"
#include "pb_proto.h"
#include "pb_util.h"
#include "param.h"
#include "stats.h"
#include "main.h"
#include "cmd.h"
#include "timer.h"
#include "net/net.h"
#include "pkt_buf.h"
#include "dump.h"

static u08 toggle_request;
static u08 auto_mode;
static u08 silent_mode;

// ----- Packet Callbacks -----

static u08 fill_pkt(u08 *buf, u16 max_size, u16 *size)
{
  *size = param.test_plen;
  if(*size > max_size) {
    return PBPROTO_STATUS_PACKET_TOO_LARGE;
  }

  net_copy_mac(net_bcast_mac, buf);
  net_copy_mac(param.mac_addr, buf+6);

  u08 ptype_hi = (u08)(param.test_ptype >> 8);
  u08 ptype_lo = (u08)(param.test_ptype & 0xff);
  buf[12] = ptype_hi;
  buf[13] = ptype_lo;

  u08 *ptr = buf + 14;
  u16 num = *size - 14;
  u08 val = 0;
  while(num > 0) {
    *ptr = val;
    ptr++;
    val++;
    num--;
  }

  return PBPROTO_STATUS_OK;  
}

static u08 proc_pkt(const u08 *buf, u16 size)
{
  u16 errors = 0;

  // check packet size
  if(size != param.test_plen) {
    errors = 1;
    uart_send_pstring(PSTR("ERR: size\r\n"));
  }

  // +0: check dst mac
  if(!net_compare_mac(buf, net_bcast_mac)) {
    errors++;
    uart_send_pstring(PSTR("ERR: dst mac\r\n"));
  }
  // +6: check src mac
  if(!net_compare_mac(buf+6, param.mac_addr)) {
    errors++;
    uart_send_pstring(PSTR("ERR: src mac\r\n"));
  }
  // +12,+13: pkt type
  u08 ptype_hi = (u08)(param.test_ptype >> 8);
  u08 ptype_lo = (u08)(param.test_ptype & 0xff);
  if((buf[12] != ptype_hi) || (buf[13] != ptype_lo)) {
    errors++;
    uart_send_pstring(PSTR("ERR: pkt type\r\n"));
  }

  // +14: data
  const u08 *ptr = buf + 14;
  u16 num = size - 14;
  u08 val = 0;
  while(num > 0) {
    if(*ptr != val) {
      uart_send_pstring(PSTR("ERR: data @"));
      uart_send_hex_word(num);
      uart_send_crlf();
    }
    val++;
    ptr++;
    num--;
  }

#if 0
  for(int i=0;i<16;i++) {
    uart_send_hex_byte(buf[i]);
    uart_send_spc();
  }
  uart_send_crlf();
#endif

  if(errors > 0) {
    uart_send_pstring(PSTR("TOTAL ERRORS "));
    uart_send_hex_word(errors);
    uart_send_crlf();
    return PBPROTO_STATUS_ERROR;
  } else {
    return PBPROTO_STATUS_OK;
  }
}

// ----- function table -----

static void pb_test_worker(void)
{
  u08 status = pb_util_handle();

  // ok!
  if(status == PBPROTO_STATUS_OK) {

    // always dump I/O
    if(!silent_mode) {
      dump_pb_cmd(&pb_proto_stat);
    }

    // next iteration?
    if(pb_proto_stat.is_send) {
      if(auto_mode) {
        // next iteration after 
        pb_test_send_packet(1);
      } else {
        silent_mode = 0;
      }
    }
  }
  // pb proto failed with an error
  else if(status != PBPROTO_STATUS_IDLE) {
    // disable auto mode
    if(auto_mode) {
      pb_test_toggle_auto();
    }
  }
}

u08 pb_test_loop(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[PB_TEST] on\r\n"));

  stats_reset();

  // setup handlers for pb testing
  pb_proto_init(fill_pkt, proc_pkt, pkt_buf, PKT_BUF_SIZE);
  auto_mode = 0;
  toggle_request = 0;
  silent_mode = 0;

  // test loop
  u08 result = CMD_WORKER_IDLE;
  while(run_mode == RUN_MODE_PB_TEST) {
    // command line handling
    result = cmd_worker();
    if(result & CMD_WORKER_RESET) {
      break;
    }

    pb_test_worker();
  }

  stats_dump(1,0);

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[PB_TEST] off\r\n"));

  return result;
}

void pb_test_send_packet(u08 silent)
{
  silent_mode = silent;
  pb_proto_request_recv();
}

void pb_test_toggle_auto(void)
{
  auto_mode = !auto_mode;

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[AUTO] "));
  if(auto_mode) {
    uart_send_pstring(PSTR("on"));
  } else {
    uart_send_pstring(PSTR("off"));
  }
  uart_send_crlf();

  if(auto_mode) {
    // send first packet
    pb_test_send_packet(1);
    // clear stats
    stats_reset();
  }
}
