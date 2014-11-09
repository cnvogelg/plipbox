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

#include "pb_proto.h"
#include "par_low.h"
#include "timer.h"

#include "uartutil.h"

// define symbolic names for protocol
#define SET_RAK         par_low_set_busy_hi
#define CLR_RAK         par_low_set_busy_lo
#define GET_REQ         par_low_get_pout
#define GET_SELECT      par_low_get_select

// recv funcs
static pb_proto_funcs_t    *funcs;

u16 pb_proto_timeout = 5000; // = 500ms in 100us ticks

// ----- Init -----

void pb_proto_init(pb_proto_funcs_t *f)
{
  funcs = f;

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
   
  // --- size hi ---
  status = wait_req(1, PBPROTO_STAGE_SIZE_HI);
  if(status != PBPROTO_STATUS_OK) {
    return status;
  }
  hi = par_low_data_in();
  CLR_RAK();
   
  // --- size lo ---
  status = wait_req(0, PBPROTO_STAGE_SIZE_LO);
  if(status != PBPROTO_STATUS_OK) {
    return status;
  }
  lo = par_low_data_in();
  SET_RAK();
   
  u16 size = hi << 8 | lo;
   
  // report size at begin of frame
  funcs->send_begin(&size);

  // round to even and convert to words
  u16 words = size;
  if(words & 1) {
    words++;
  }
  words >>= 1;
  
  // --- data loop ---
  u16 i;
  u08 data;
  u16 got = 0;
  for(i = 0; i < words; i++) {
    // -- even byte: 0,2,4,...
    status = wait_req(1, PBPROTO_STAGE_DATA);
    if(status != PBPROTO_STATUS_OK) {
      break;
    }
    data = par_low_data_in();
    funcs->send_data(&data);
    CLR_RAK();
    got++;

    // -- odd byte: 1,3,5,...
    status = wait_req(0, PBPROTO_STAGE_DATA);
    if(status != PBPROTO_STATUS_OK) {
      break;
    }
    data = par_low_data_in();
    funcs->send_data(&data);
    SET_RAK();
    got++;
  }
  
  // report end of frame
  funcs->send_end(size);
  
  *ret_size = got;
  return status;
}

// amiga wants to receive a packet
static u08 cmd_recv(u16 *ret_size)
{
  u16 size;
  
  // ask for size
  funcs->recv_begin(&size);
  *ret_size = size;
    
  // --- size hi ----
  u08 status = wait_req(1, PBPROTO_STAGE_SIZE_HI);
  if(status != PBPROTO_STATUS_OK) {
    return status;
  }
  u08 hi = (u08)(size >> 8);
  
  // [OUT]
  par_low_data_set_output();

  par_low_data_out(hi);
  CLR_RAK();
  
  // --- size lo ---
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
  for(u16 i = 0; i < words; i++) {
    // even bytes 0,2,4,...
    status = wait_req(1, PBPROTO_STAGE_DATA);
    if(status != PBPROTO_STATUS_OK) {
      break;
    }
    u08 data;
    funcs->recv_data(&data);
    par_low_data_out(data);
    CLR_RAK();
    got++;

    // odd bytes 1,3,5,...
    status = wait_req(0, PBPROTO_STAGE_DATA);
    if(status != PBPROTO_STATUS_OK) {
      break;
    }
    funcs->recv_data(&data);
    par_low_data_out(data);
    SET_RAK();
    got++;
  }
  
  // final wait
  if(status == PBPROTO_STATUS_OK) {
    status = wait_req(1, PBPROTO_STAGE_LAST_DATA);
  }
  
  // [IN]
  par_low_data_set_input();
  
  funcs->recv_end(size);
  
  *ret_size = got;
  return status;
}

// ---------- BURST ----------

#define MAX_BURST_BYTES   768
#define MAX_BURST_WORDS   (MAX_BURST_BYTES / 2)

static u08 buffer[MAX_BURST_BYTES];

static u08 cmd_send_burst(u16 *ret_size)
{
  u08 hi, lo, bhi, blo;
  u08 status;
   
  // --- burst size hi ---
  status = wait_req(1, PBPROTO_STAGE_BURST_HI);
  if(status != PBPROTO_STATUS_OK) {
    return status;
  }
  bhi = par_low_data_in();
  CLR_RAK();
   
  // --- burst size lo ---
  status = wait_req(0, PBPROTO_STAGE_BURST_LO);
  if(status != PBPROTO_STATUS_OK) {
    return status;
  }
  blo = par_low_data_in();
  SET_RAK();
   
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
  u16 burst_size = bhi << 8 | blo; // size in words -1 !
  burst_size++; // now in words

  // check burst size
  if(burst_size >= MAX_BURST_WORDS) {
    return PBPROTO_STATUS_BURST_TOO_LARGE;
  }
   
  // round to even and convert to words
  u16 words = size;
  if(words & 1) {
    words++;
  }
  words >>= 1;

  funcs->send_begin(&size);

  // ----- burst chunk loop -----
  u16 i;
  u08 result = PBPROTO_STATUS_OK;
  u16 got_words = 0;
  while(words > 0) {

    // calc size of burst
    u16 bs = burst_size;
    if(bs > words) {
      bs = words;
    }
    words -= bs;
    u08 *ptr = buffer;

    // ----- burst loop -----
    // BEGIN TIME CRITICAL
    cli();
    SET_RAK(); // trigger start of burst
    for(i=0;i<bs;i++) {
      // wait REQ == 1
      while(!GET_REQ()) {
        if(!GET_SELECT()) goto send_burst_exit;
      }
      *(ptr++) = par_low_data_in();
      
      // wait REQ == 0
      while(GET_REQ()) {
        if(!GET_SELECT()) goto send_burst_exit;
      }
      *(ptr++) = par_low_data_in();
    }
send_burst_exit:
    CLR_RAK();
    sei();
    // END TIME CRITICAL
  
    got_words += i;

    // error?
    if(i<bs) {
      result = PBPROTO_STATUS_TIMEOUT | PBPROTO_STAGE_DATA;
      break;
    }
 
    // push to eth
    // TODO: use new data_block func
    ptr = buffer;
    for(i=0;i<bs;i++) {
      funcs->send_data(ptr++);
      funcs->send_data(ptr++);
    }
  }

  // final ACK 
  SET_RAK();

  funcs->send_end(size);

  *ret_size = got_words << 1;
  return result;  
}

static u08 cmd_recv_burst(u16 *ret_size)
{
  u08 hi, lo, bhi, blo;
  u08 status;
   
  // ask for size
  u16 size;
  funcs->recv_begin(&size);

  hi = (u08)(size >> 8);
  lo = (u08)(size & 0xff);

  // --- get burst size hi ---
  status = wait_req(1, PBPROTO_STAGE_BURST_HI);
  if(status != PBPROTO_STATUS_OK) {
    return status;
  }
  bhi = par_low_data_in();
  CLR_RAK();
   
  // --- get burst size lo ---
  status = wait_req(0, PBPROTO_STAGE_BURST_LO);
  if(status != PBPROTO_STATUS_OK) {
    return status;
  }
  blo = par_low_data_in();
  SET_RAK();
   
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

  // calc burst size
  u16 burst_size = bhi << 8 | blo; // size in words -1 !
  burst_size++; // now in words

  // check burst size
  if(burst_size >= MAX_BURST_WORDS) {
    return PBPROTO_STATUS_BURST_TOO_LARGE;
  }
   
  // round to even and convert to words
  u16 words = size;
  if(words & 1) {
    words++;
  }
  words >>= 1;

  uart_send_hex_word(burst_size);
  uart_send_crlf();
  uart_send_hex_word(words);
  uart_send_crlf();

  // ----- burst chunk loop -----
  u16 i;
  u08 result = PBPROTO_STATUS_OK;
  u16 got_words = 0;
  while(words > 0) {

    // calc size of burst
    u16 bs = burst_size;
    if(bs > words) {
      bs = words;
    }
    words -= bs;
    u08 *ptr = buffer;

    // fill buffer
    for(i=0;i<bs;i++) {
      funcs->recv_data(ptr++);
      funcs->recv_data(ptr++);
    }
    ptr = buffer;

    // ----- burst loop -----
    // BEGIN TIME CRITICAL
    cli();
    CLR_RAK(); // trigger start of burst
    for(i=0;i<bs;i++) {
      par_low_data_out(*(ptr++));
      // wait REQ == 0
      while(GET_REQ()) {
        if(!GET_SELECT()) goto recv_burst_exit;
      }

      par_low_data_out(*(ptr++));      
      // wait REQ == 1
      while(!GET_REQ()) {
        if(!GET_SELECT()) goto recv_burst_exit;
      }
    }
recv_burst_exit:
    SET_RAK();
    sei();
    // END TIME CRITICAL
  
    got_words += i;

    // error?
    if(i<bs) {
      result = PBPROTO_STATUS_TIMEOUT | PBPROTO_STAGE_DATA;
      break;
    }
  }

  // final ACK 
  CLR_RAK();

  // [IN]
  par_low_data_set_input();

  funcs->recv_end(size);

  *ret_size = got_words << 1;
  return result;  
}

u08 pb_proto_handle(u08 *ret_cmd, u16 *ret_size)
{
  u08 result;
   
  // handle server side of plipbox protocol
  *ret_cmd = 0; 
  
  // make sure that SEL == 1
  if(!GET_SELECT()) {
    return PBPROTO_STATUS_IDLE;
  }
  
  // make sure that REQ == 0
  if(GET_REQ()) {
    return PBPROTO_STATUS_IDLE;
  }
  
  // read command byte and execute it
  u08 cmd = par_low_data_in();

  // confirm cmd with RAK = 1
  SET_RAK();

  switch(cmd) {
    case PBPROTO_CMD_RECV:
      result = cmd_recv(ret_size);
      break;
    case PBPROTO_CMD_SEND:
      result = cmd_send(ret_size);
      break;
    case PBPROTO_CMD_SEND_BURST:
      result = cmd_send_burst(ret_size);
      break;
    case PBPROTO_CMD_RECV_BURST:
      result = cmd_recv_burst(ret_size);
      break;
    default:
      result = PBPROTO_STATUS_INVALID_CMD;
      break;
  }
   
  // wait for SEL == 0
  wait_sel(0, PBPROTO_STAGE_END_SELECT);
  
  // reset RAK = 0
  CLR_RAK();
   
  *ret_cmd = cmd;
  return result;
}
