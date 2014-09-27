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

#include "pb_proto.h"
#include "par_low.h"
#include "timer.h"

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
  
  // --- data loop ---
  u08 toggle = 1;
  u16 i;
  for(i = 0; i < size; i++) {
    status = wait_req(toggle, PBPROTO_STAGE_DATA);
    if(status != PBPROTO_STATUS_OK) {
      break;
    }
    u08 data = par_low_data_in();
    funcs->send_data(&data);
    if(toggle) {
      CLR_RAK();
    } else {
      SET_RAK();
    }
    toggle = !toggle;
  }
  *ret_size = i;
  
  // report end of frame
  funcs->send_end(size);
  
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
  
  // --- data ---
  u08 toggle = 1;
  for(u16 i = 0; i < size; i++) {
    status = wait_req(toggle, PBPROTO_STAGE_DATA);
    if(status != PBPROTO_STATUS_OK) {
      break;
    }
    u08 data;
    funcs->recv_data(&data);
    par_low_data_out(data);
    if(toggle) {
      CLR_RAK();
    } else {
      SET_RAK();
    }
    toggle = !toggle;
  }
  
  // final wait
  if(status == PBPROTO_STATUS_OK) {
    status = wait_req(toggle, PBPROTO_STAGE_LAST_DATA);
  }
  
  // [IN]
  par_low_data_set_input();
  
  funcs->recv_end(size);
  
  return status;
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
