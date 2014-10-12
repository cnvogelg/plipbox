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
#include "uart.h"
#include "eth_state.h"
#include "pb_state.h"
#include "pb_io.h"
#include "pb_proto.h"
#include "param.h"
#include "timer.h"

#define TEST_STATE_OFF    0
#define TEST_STATE_ENTER  1
#define TEST_STATE_ACTIVE 2
#define TEST_STATE_LEAVE  3

static u32 start_ts;
static u32 trigger_ts;
static u16 count;
static u16 errors;

static u08 toggle_request;
static u08 state = TEST_STATE_OFF;


// ----- RX Calls -----

static void rx_begin(u16 *pkt_size)
{
  start_ts = time_stamp;
  errors = 0;
  count = 0;

  // check packet size
  if(*pkt_size != param.test_plen) {
    errors = 1;
  }
}

static void rx_data(u08 *data)
{
  // check dst mac
  if(count < 6) {
    if(*data != 0xff) {
      errors++;
    }
  }
  // check src mac
  else if(count < 12) {
    u08 mac_byte = param.mac_addr[count-6];
    if(*data != mac_byte) {
      errors++;
    }
  }
  // check type
  else if(count == 12) {
    u08 ptype_hi = (u08)(param.test_ptype >> 8);
    if(*data != ptype_hi) {
      errors++;
    }
  }
  else if(count == 13) {
    u08 ptype_lo = (u08)(param.test_ptype & 0xff);
    if(*data != ptype_lo) {
      errors++;
    }    
  }
  // data 
  else {
    u08 val = (u08)((count - 14) & 0xff);
    if(*data != val) {
      errors++;
    }
  }
  count++;
}

static void rx_end(u16 pkt_size)
{
  u32 end_ts = time_stamp;
  u32 delta = end_ts - start_ts;

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[RX] "));
  uart_send_time_stamp_spc_ext(delta);
  uart_send_hex_word(pkt_size);
  uart_send_spc();
  uart_send_hex_word(errors);
  uart_send_crlf();
}

// ----- TX Calls -----

static void tx_begin(u16 *pkt_size)
{
  start_ts = time_stamp;
  trigger_ts = start_ts - trigger_ts;
  *pkt_size = param.test_plen;
  count = 0;
}

static void tx_data(u08 *data)
{
  // dst mac
  if(count < 6) {
    *data = 0xff; // broadcast
  }
  // src mac
  else if(count < 12) {
    u08 mac_byte = param.mac_addr[count-6];
    *data = mac_byte; // my mac
  }
  // type
  else if(count == 12) {
    u08 ptype_hi = (u08)(param.test_ptype >> 8);
    *data = ptype_hi;
  }
  else if(count == 13) {
    u08 ptype_lo = (u08)(param.test_ptype & 0xff);
    *data = ptype_lo;
  }
  // data
  else {
    *data = (u08)((count - 14) & 0xff);
  }
  count ++;
}

static void tx_end(u16 pkt_size)
{
  u32 end_ts = time_stamp;
  u32 delta = end_ts - start_ts;

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[TX] "));
  uart_send_time_stamp_spc_ext(delta);
  uart_send_time_stamp_spc_ext(trigger_ts);
  uart_send_hex_word(pkt_size);
  uart_send_crlf();  
}

// ----- function table -----

static pb_proto_funcs_t funcs = {
  .send_begin = rx_begin,
  .send_data = rx_data,
  .send_end = rx_end,
  
  .recv_begin = tx_begin,
  .recv_data = tx_data,
  .recv_end = tx_end
};

// ----- API -----

void pb_test_toggle_mode(void)
{
  toggle_request = 1;
}

void pb_test_send_packet(void)
{
  if(state != TEST_STATE_ACTIVE) {
    return;
  }

  trigger_ts = time_stamp;
  pb_proto_request_recv();
}

static void dump_state(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[TEST] "));
  
  PGM_P str = 0;
  switch(state) {
    case TEST_STATE_OFF:
      str = PSTR("off");
      break;
    case TEST_STATE_ENTER:
      str = PSTR("enter");
      break;
    case TEST_STATE_ACTIVE:
      str = PSTR("active");
      break;
    case TEST_STATE_LEAVE:
      str = PSTR("leave");
      break;
  }

  uart_send_pstring(str);
  uart_send_crlf();
}

u08 pb_test_state(u08 eth_state, u08 pb_state)
{
  switch(state) {
    case TEST_STATE_OFF:
      if(toggle_request) {
        // disable ethernet
        eth_state_shutdown();

        state = TEST_STATE_ENTER;
        dump_state();
        toggle_request = 0;
      }
      break;
    case TEST_STATE_ENTER:
      // wait for ethernet off, but required pb link
      if((eth_state == ETH_STATE_OFF) && (pb_state == PB_STATE_LINK_UP)) {
        // setup handlers for pb testing
        pb_proto_init(&funcs);

        state = TEST_STATE_ACTIVE;
        dump_state();
      }
      // skip to active if abort was toggled
      if(toggle_request) {
        state = TEST_STATE_ACTIVE;
      }
      break;
    case TEST_STATE_ACTIVE:
      if(toggle_request) {
        // enable ethernet
        eth_state_init();
        // restore plibbox io handler
        pb_io_init();

        state = TEST_STATE_LEAVE;
        dump_state();
        toggle_request = 0;
      }
      break;
    case TEST_STATE_LEAVE:
      // wait for ethernet on again
      if((eth_state == ETH_STATE_LINK_DOWN)||(eth_state == ETH_STATE_LINK_UP)) {
        state = TEST_STATE_OFF;
        dump_state();
      }
  }
  return state == TEST_STATE_ACTIVE;
}

void pb_test_worker(void)
{

}
