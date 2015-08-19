/*
 * pb_proto.c - avr implementation of the plipbox protocol
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

#include <avr/interrupt.h>
#include <util/delay_basic.h>

#include "pb_proto.h"
#include "par_low.h"
#include "timer.h"
#include "stats.h"

#include "uartutil.h"

// define symbolic names for protocol
#define SET_RAK         par_low_set_busy_hi
#define CLR_RAK         par_low_set_busy_lo
#define GET_REQ         par_low_get_pout
#define GET_SELECT      par_low_get_select
#define GET_STROBE      par_low_get_strobe

// recv funcs
static pb_proto_fill_func fill_func;
static pb_proto_proc_func proc_func;
static u08 *pb_buf;
static u16 pb_buf_size;
static u32 trigger_ts;

u16 pb_proto_timeout = 5000; // = 500ms in 100us ticks

// public stat func
pb_proto_stat_t pb_proto_stat;

// ----- Init -----

void pb_proto_init(pb_proto_fill_func ff, pb_proto_proc_func pf, u08 *buf, u16 buf_size)
{
  fill_func = ff;
  proc_func = pf;
  pb_buf = buf;
  pb_buf_size = buf_size;

  // init signals
  par_low_data_set_input();
  CLR_RAK();
}

u08 pb_proto_get_line_status(void)
{
  u08 strobe = par_low_get_strobe();
  u08 select = par_low_get_select();
  u08 pout = par_low_get_pout();
  u08 status = 0;
  if(strobe) {
    status |= 0x1;
  }
  if(select) {
    status |= 0x2;
  }
  if(pout) {
    status |= 0x4;
  }
  return status;
}

void pb_proto_request_recv(void)
{
  par_low_pulse_ack(1);
  trigger_ts = time_stamp;
}

// ----- HELPER -----

static u08 wait_req(u08 toggle_expect, u08 state_flag)
{
  // wait for new REQ value
  timer_100us = 0;
  while(timer_100us < pb_proto_timeout) {
    u08 pout = GET_REQ();
    if((toggle_expect && pout) || (!toggle_expect && !pout)) {
      return PBPROTO_STATUS_OK;
    }
    // during transfer client aborted and removed SEL
    u08 select = GET_SELECT();
    if(!select) {
      return PBPROTO_STATUS_LOST_SELECT | state_flag;
    }
  }
  return PBPROTO_STATUS_TIMEOUT | state_flag;
}

static u08 wait_sel(u08 select_state, u08 state_flag)
{
  timer_100us = 0;
  while(timer_100us < pb_proto_timeout) {
    if(GET_SELECT() == select_state) {
      return PBPROTO_STATUS_OK;
    }
  }
  return PBPROTO_STATUS_TIMEOUT | state_flag;
}

// ---------- Handler ----------

// amiga wants to send a packet
static u08 cmd_send(u16 *ret_size)
{
  u08 hi, lo;
  u08 status;

  // --- get size hi ---
  status = wait_req(1, PBPROTO_STAGE_SIZE_HI);
  if(status != PBPROTO_STATUS_OK) {
    return status;
  }
  hi = par_low_data_in();
  CLR_RAK();

  // --- get size lo ---
  status = wait_req(0, PBPROTO_STAGE_SIZE_LO);
  if(status != PBPROTO_STATUS_OK) {
    return status;
  }
  lo = par_low_data_in();
  SET_RAK();

  u16 size = hi << 8 | lo;

  // check size
  if(size > pb_buf_size) {
    return PBPROTO_STATUS_PACKET_TOO_LARGE;
  }

  // round to even and convert to words
  u16 words = size;
  if(words & 1) {
    words++;
  }
  words >>= 1;

  // --- data loop ---
  u16 i;
  u16 got = 0;
  u08 *ptr = pb_buf;
  for(i = 0; i < words; i++) {
    // -- even byte: 0,2,4,...
    status = wait_req(1, PBPROTO_STAGE_DATA);
    if(status != PBPROTO_STATUS_OK) {
      break;
    }
    *(ptr++) = par_low_data_in();
    CLR_RAK();
    got++;

    // -- odd byte: 1,3,5,...
    status = wait_req(0, PBPROTO_STAGE_DATA);
    if(status != PBPROTO_STATUS_OK) {
      break;
    }
    *(ptr++) = par_low_data_in();
    SET_RAK();
    got++;
  }

  *ret_size = got;
  return status;
}

// amiga wants to receive a packet
static u08 cmd_recv(u16 size, u16 *ret_size)
{
  // --- set size hi ----
  u08 status = wait_req(1, PBPROTO_STAGE_SIZE_HI);
  if(status != PBPROTO_STATUS_OK) {
    return status;
  }
  u08 hi = (u08)(size >> 8);

  // [OUT]
  par_low_data_set_output();

  par_low_data_out(hi);
  CLR_RAK();

  // --- set size lo ---
  status = wait_req(0, PBPROTO_STAGE_SIZE_LO);
  if(status != PBPROTO_STATUS_OK) {
    return status;
  }
  u08 lo = (u08)(size & 0xff);
  par_low_data_out(lo);
  SET_RAK();

  // get number of words
  u16 words = size;
  if(words & 1) {
    words++;
  }
  words >>= 1;

  // --- data ---
  u16 got = 0;
  const u08 *ptr = pb_buf;
  for(u16 i = 0; i < words; i++) {
    // even bytes 0,2,4,...
    status = wait_req(1, PBPROTO_STAGE_DATA);
    if(status != PBPROTO_STATUS_OK) {
      break;
    }
    par_low_data_out(*(ptr++));
    CLR_RAK();
    got++;

    // odd bytes 1,3,5,...
    status = wait_req(0, PBPROTO_STAGE_DATA);
    if(status != PBPROTO_STATUS_OK) {
      break;
    }
    par_low_data_out(*(ptr++));
    SET_RAK();
    got++;
  }

  // final wait
  if(status == PBPROTO_STATUS_OK) {
    status = wait_req(1, PBPROTO_STAGE_LAST_DATA);
  }

  // [IN]
  par_low_data_set_input();

  *ret_size = got;
  return status;
}

// ---------- BURST ----------

static u08 cmd_send_burst(u16 *ret_size)
{
  u08 hi, lo;
  u08 status;

  // --- packet size hi ---
  status = wait_req(1, PBPROTO_STAGE_SIZE_HI);
  if(status != PBPROTO_STATUS_OK) {
    return status;
  }
  hi = par_low_data_in();
  CLR_RAK();

  // --- packet size lo ---
  status = wait_req(0, PBPROTO_STAGE_SIZE_LO);
  if(status != PBPROTO_STATUS_OK) {
    return status;
  }
  lo = par_low_data_in();
  // delay SET_RAK until burst begin...

  u16 size = hi << 8 | lo;

  // check size
  if(size > pb_buf_size) {
    return PBPROTO_STATUS_PACKET_TOO_LARGE;
  }

  // round to even and convert to words
  if(size & 1) {
    size++;
  }
  u16 i;
  u08 result = PBPROTO_STATUS_OK;
  u08 *ptr = pb_buf;

  // ----- burst loop -----
  // BEGIN TIME CRITICAL
  cli();
  SET_RAK(); // trigger start of burst
  for(i=0;i<size;i++) {
    // wait STROBE falling edge
    while(GET_STROBE()) {
      if(!GET_SELECT()) goto send_burst_exit;
    }
    *(ptr++) = par_low_data_in();
    // wait end of STROBE
    while(!GET_STROBE()) {
      if(!GET_SELECT()) goto send_burst_exit;
    }
  }
send_burst_exit:
  sei();
  // END TIME CRITICAL

  // wait REQ == 1
  while(!GET_REQ()) {
    if(!GET_SELECT()) goto send_burst_exit;
  }

  CLR_RAK();

  // wait REQ == 0
  while(GET_REQ()) {
    if(!GET_SELECT()) goto send_burst_exit;
  }

  // error?
  if(i<size) {
    result = PBPROTO_STATUS_TIMEOUT | PBPROTO_STAGE_DATA;
  }

  // final ACK
  SET_RAK();

  *ret_size = i;
  return result;
}

static u08 cmd_recv_burst(u16 size, u16 *ret_size)
{
  u08 hi, lo;
  u08 status;

  hi = (u08)(size >> 8);
  lo = (u08)(size & 0xff);

  // --- set packet size hi
  status = wait_req(1, PBPROTO_STAGE_SIZE_HI);
  if(status != PBPROTO_STATUS_OK) {
    return status;
  }
  par_low_data_set_output();
  par_low_data_out(hi);
  CLR_RAK();

  // --- set packet size lo ---
  status = wait_req(0, PBPROTO_STAGE_SIZE_LO);
  if(status != PBPROTO_STATUS_OK) {
    return status;
  }
  par_low_data_out(lo);
  SET_RAK();

  // --- burst ready? ---
  status = wait_req(1, PBPROTO_STAGE_DATA);
  if(status != PBPROTO_STATUS_OK) {
    return status;
  }

  // round to even and convert to words
  u08 result = PBPROTO_STATUS_OK;
  u16 i;
  u08 *ptr = pb_buf;
  if((size&1)==0) {
    size--;
  }

  // ----- burst loop -----
  // BEGIN TIME CRITICAL
  cli();
  // prepare first byte
  par_low_data_out(*(ptr++));
  CLR_RAK(); // trigger start of burst
  // loop
  for(i=0;i<size;i++) {
    // wait for STROBE==0
    while(GET_STROBE()) {
      if(!GET_SELECT()) goto recv_burst_exit;
    }
    par_low_data_out(*(ptr++));
    // wait for STROBE==1 again
    while(!GET_STROBE()) {
      if(!GET_SELECT()) goto recv_burst_exit;
    }
  }
recv_burst_exit:
  sei();
  // END TIME CRITICAL

  // final wait REQ == 0
  while(GET_REQ()) {
    if(!GET_SELECT()) goto recv_burst_exit;
  }

  SET_RAK();

  // final wait REQ == 1
  while(!GET_REQ()) {
    if(!GET_SELECT()) goto recv_burst_exit;
  }

  // error?
  if(i<size) {
    result = PBPROTO_STATUS_TIMEOUT | PBPROTO_STAGE_DATA;
  }

  // final ACK
  CLR_RAK();

  // [IN]
  par_low_data_set_input();

  *ret_size = i+1;
  return result;
}

u08 pb_proto_handle(void)
{
  u08 result;
  pb_proto_stat_t *ps = &pb_proto_stat;

  // handle server side of plipbox protocol
  ps->cmd = 0;

  // make sure that SEL == 1
  if(!GET_SELECT()) {
    ps->status = PBPROTO_STATUS_IDLE;
    return PBPROTO_STATUS_IDLE;
  }

  // make sure that REQ == 0
  if(GET_REQ()) {
    ps->status = PBPROTO_STATUS_IDLE;
    return PBPROTO_STATUS_IDLE;
  }

  // read command byte and execute it
  u08 cmd = par_low_data_in();

  // fill buffer for recv command
  u16 pkt_size = 0;
  if((cmd == PBPROTO_CMD_RECV) || (cmd == PBPROTO_CMD_RECV_BURST)) {
    u08 res = fill_func(pb_buf, pb_buf_size, &pkt_size);
    if(res != PBPROTO_STATUS_OK) {
      ps->status = res;
      return res;
    }
  }

  // start timer
  u32 ts = time_stamp;
  timer_hw_reset();

  // confirm cmd with RAK = 1
  SET_RAK();

  u16 ret_size = 0;
  switch(cmd) {
    case PBPROTO_CMD_RECV:
      result = cmd_recv(pkt_size, &ret_size);
      break;
    case PBPROTO_CMD_SEND:
      result = cmd_send(&ret_size);
      break;
    case PBPROTO_CMD_RECV_BURST:
      result = cmd_recv_burst(pkt_size, &ret_size);
      break;
    case PBPROTO_CMD_SEND_BURST:
      result = cmd_send_burst(&ret_size);
      break;
    default:
      result = PBPROTO_STATUS_INVALID_CMD;
      break;
  }

  // wait for SEL == 0
  wait_sel(0, PBPROTO_STAGE_END_SELECT);

  // reset RAK = 0
  CLR_RAK();

  // read timer
  u16 delta = timer_hw_get();

  // process buffer for send command
  if(result == PBPROTO_STATUS_OK) {
    if((cmd == PBPROTO_CMD_SEND) || (cmd == PBPROTO_CMD_SEND_BURST)) {
      result = proc_func(pb_buf, ret_size);
    }
  }

  // fill in stats
  ps->cmd = cmd;
  ps->status = result;
  ps->size = ret_size;
  ps->delta = delta;
  ps->rate = timer_hw_calc_rate_kbs(ret_size, delta);
  ps->ts = ts;
  ps->is_send = (cmd == PBPROTO_CMD_SEND) || (cmd == PBPROTO_CMD_SEND_BURST);
  ps->stats_id = ps->is_send ? STATS_ID_PB_TX : STATS_ID_PB_RX;
  ps->recv_delta = ps->is_send ? 0 : (u16)(ps->ts - trigger_ts);
  return result;
}
