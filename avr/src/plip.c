/*
 * plip.c - avr implementation of magPLIP protocol
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

#include "plip.h"
#include "par_low.h"
#include "timer.h"

#define SET_REQ         par_low_set_busy_hi
#define CLR_REQ         par_low_set_busy_lo
#define TEST_LINE       par_low_get_pout
#define TEST_SELECT     par_low_get_select

// recv funcs
static plip_packet_func   begin_rx_func = 0;
static plip_data_func     fill_rx_func = 0;
static plip_packet_func   end_rx_func = 0;

// send funcs
static plip_data_func     fill_tx_func = 0;

u16 plip_timeout = 5000; // = 500ms in 100us ticks

void plip_recv_init(plip_packet_func begin_func, 
                    plip_data_func   fill_func,
                    plip_packet_func end_func)
{
  begin_rx_func = begin_func;
  fill_rx_func  = fill_func;
  end_rx_func   = end_func;
}

u08 plip_get_line_status(void)
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

void plip_send_init(plip_data_func   fill_func)
{
  fill_tx_func  = fill_func;
}

static u08 wait_line_toggle(u08 toggle_expect, u08 state_flag)
{
  // wait for new toggle value
  timer_100us = 0;
  while(timer_100us < plip_timeout) {
    u08 pout = TEST_LINE();
    if((toggle_expect && pout) || (!toggle_expect && !pout)) {
      return PLIP_STATUS_OK;
    }
    // during transfer peer switched from rx to tx or vice versa -> arbitration lost!
    u08 select = TEST_SELECT();
    if(!select) {
      return PLIP_STATUS_LOST_SELECT | state_flag;
    }
  }
  return PLIP_STATUS_TIMEOUT | state_flag;
}

// ---------- Receive ----------

static u08 get_next_byte(u08 *data, u08 toggle_expect, u08 state_flag)
{
  // wait for HS_LINE toggle
  u08 status = wait_line_toggle(toggle_expect, state_flag);
  if(status == PLIP_STATUS_OK) {
    // read byte
    *data = par_low_data_in();
    
    // toggle HS_REQUEST (BUSY)
    par_low_toggle_busy();
  }
  return status;
}

static u08 get_next_word(u16 *data, u08 toggle_expect, u08 state_flag)
{
  u08 a,b;
  u08 status = get_next_byte(&a, toggle_expect, state_flag);
  if(status != PLIP_STATUS_OK) {
    return status;
  }
  status = get_next_byte(&b, !toggle_expect, state_flag);
  if(status != PLIP_STATUS_OK) {
    return status;
  }
  *data = ((u16)a) << 8 | b;
  return PLIP_STATUS_OK;
}

static u08 get_next_bytes(u08 *data, u08 size, u08 toggle_expect, u08 state_flag)
{
  for(u08 i=0;i<size;i++) {
    u08 status = get_next_byte(&data[i], toggle_expect, state_flag);
    if(status != PLIP_STATUS_OK) {
      return status;
    }
    toggle_expect = !toggle_expect;
  }
  return PLIP_STATUS_OK;
}

static u08 wait_for_select(u08 select_state, u08 state_flag)
{
  timer_100us = 0;
  while(timer_100us < plip_timeout) {
    if(TEST_SELECT() == select_state) {
      return PLIP_STATUS_OK;
    }
  }
  return PLIP_STATUS_TIMEOUT | state_flag;
}

u08 plip_can_recv(void)
{
  // check PTRSEL -> is 1=write loop in magplip (SETCIAOUTPUT)
  if(!TEST_SELECT()) {
    return PLIP_STATUS_IDLE;
  }
  
  // if LINE is lo then peer did not start send
  if(!TEST_LINE() || (par_low_strobe_count == 0)) {
    return PLIP_STATUS_IDLE;
  }

  return PLIP_STATUS_OK;
}

u08 plip_recv(plip_packet_t *pkt)
{
  u08 status = PLIP_STATUS_OK;

  pkt->real_size = 0;
  
  // first byte must be magic (0x42) 
  // this byte was set by last strobe received before calling this func
  u08 magic = 0;
  status = get_next_byte(&magic, 1, PLIP_STATE_MAGIC);
  if(status == PLIP_STATUS_OK) {
    if(magic != PLIP_MAGIC) {
      status = PLIP_STATUS_NO_MAGIC;
    }
  }

  // expect CRC type
  u08 crc_type = 0;
  if(status == PLIP_STATUS_OK) {
    status = get_next_byte(&crc_type, 0, PLIP_STATE_CRC_TYPE);
  }
  if(status == PLIP_STATUS_OK) {
    // make sure its a valid crc_type
    if((crc_type != PLIP_CRC) && (crc_type != PLIP_NOCRC)) {
      status = PLIP_STATUS_INVALID_MAGIC;
    } else {
      pkt->crc_type = crc_type;
    }
  }
  
  // expect size
  u16 size;
  if(status == PLIP_STATUS_OK) {
    status = get_next_word(&size, 1, PLIP_STATE_SIZE);
  }
  
  // read crc
  u16 crc;
  if(status == PLIP_STATUS_OK) {
    pkt->size = size - 6;
    status = get_next_word(&crc, 1, PLIP_STATE_CRC);
  }
  size -= 2;
  
  // read type
  u16 type = 0;
  if(status == PLIP_STATUS_OK) {
    pkt->crc = crc;
    status = get_next_word(&type, 1, PLIP_STATE_TYPE);
  }
  size -= 4;
  
  // read src addr
  if(status == PLIP_STATUS_OK) {
    pkt->type = type;
    status = get_next_bytes(pkt->src_addr, PLIP_ADDR_SIZE, 1, PLIP_STATE_SRC_ADDR);
  }
  size -= PLIP_ADDR_SIZE;
  
  // read tgt addr
  if(status == PLIP_STATUS_OK) {
    status = get_next_bytes(pkt->dst_addr, PLIP_ADDR_SIZE, 1, PLIP_STATE_DST_ADDR);
  }
  size -= PLIP_ADDR_SIZE;
  
  // report begin of frame
  if(status == PLIP_STATUS_OK) {
    status = begin_rx_func(pkt);
  }
  
  // read data
  if(status == PLIP_STATUS_OK) {
    u08 expect_toggle = 1;
    u16 i;
    for(i=0;i<size;i++) {
      u08 b;
      status = get_next_byte(&b, expect_toggle, PLIP_STATE_DATA);
      if(status != PLIP_STATUS_OK) {
        break;
      }
    
      // report to fill frame func
      status = fill_rx_func(&b);
      if(status != PLIP_STATUS_OK) {
        break;
      }
      expect_toggle = !expect_toggle;
    }
    // update real size
    pkt->real_size = i;
  }

  // report end of frame
  if(status == PLIP_STATUS_OK) {
    status = end_rx_func(pkt);
  }

  // clear all strobes occurred during recv
  par_low_strobe_count = 0;

  // clear HS_REQUEST (BUSY) to signal end of transmission
  CLR_REQ();
  
  // wait for output state to end -> SELECT=0
  if(status == PLIP_STATUS_OK) {
    status = wait_for_select(0, PLIP_STATE_END);
  }
  
  return status;
}

// ---------- Send ----------

static u08 set_next_byte(u08 data, u08 toggle_expect, u08 state_flag)
{
  // wait for new toggle value
  u08 status = wait_line_toggle(toggle_expect, state_flag);
  if(status == PLIP_STATUS_OK) {
      // set new value
      par_low_data_out(data);
      
      // toggle BUSY
      par_low_toggle_busy();
  }
  return status;
}

static u08 set_next_word(u16 word, u08 toggle_expect, u08 state_flag)
{
  u08 a = (u08)(word >> 8);
  u08 status = set_next_byte(a, toggle_expect, state_flag);
  if(status == PLIP_STATUS_OK) {
    u08 b = (u08)(word & 0xff);
    status = set_next_byte(b, !toggle_expect, state_flag);
  }
  return status;
}

static u08 set_next_bytes(u08 *data, u08 size, u08 toggle_expect, u08 state_flag)
{
  for(u08 i=0;i<size;i++) {
    u08 status = set_next_byte(data[i], toggle_expect, state_flag);
    if(status != PLIP_STATUS_OK) {
      return status;
    }
    toggle_expect = !toggle_expect;
  }
  return PLIP_STATUS_OK;
}

u08 plip_send(plip_packet_t *pkt)
{
  u08 status = PLIP_STATUS_OK;
  pkt->real_size = 0;
  
  // did the peer already begin sending?
  if(TEST_LINE() || TEST_SELECT()) {
    // immediately start the receiption
    return PLIP_STATUS_CANT_SEND;
  }
  
  // set my HS_REQUEST line
  SET_REQ();
  
  // set data bus to output
  par_low_data_set_output();

  // set magic on data bus
  par_low_data_out(PLIP_MAGIC);
  
  // make 1us ACK pulse to amiga -> triggers CIA irq
  par_low_pulse_ack(1);
  
  // wait for amiga to enter recv loop
  status = wait_for_select(1, PLIP_STATE_START);
  
  // send crc type
  u08 crc_type = pkt->crc_type;
  status = set_next_byte(crc_type, 1, PLIP_STATE_CRC_TYPE);
  
  // send size
  if(status==PLIP_STATUS_OK) {
    // pkt size = data size + CRC (word) + TYPE (word) + SRC_ADDR + DST_ADDR
    u16 size = pkt->size + (2 + 2 + (2*PLIP_ADDR_SIZE)); 
    status = set_next_word(size, 0, PLIP_STATE_SIZE);
  }
  
  // send crc
  if(status==PLIP_STATUS_OK) {
    status = set_next_word(pkt->crc, 0, PLIP_STATE_CRC);
  }
  
  // send type
  if(status==PLIP_STATUS_OK) {
    status = set_next_word(pkt->type, 0, PLIP_STATE_TYPE);
  }
  
  // send src addr
  if(status==PLIP_STATUS_OK) {
    status = set_next_bytes(pkt->src_addr, PLIP_ADDR_SIZE, 0, PLIP_STATE_SRC_ADDR);
  }
  
  // send dst addr
  if(status==PLIP_STATUS_OK) {
    status = set_next_bytes(pkt->dst_addr, PLIP_ADDR_SIZE, 0, PLIP_STATE_SRC_ADDR);
  }

  // send packet bytes
  if(status==PLIP_STATUS_OK) {
    u08 toggle_expect = 0;
    u16 size = pkt->size;
    u16 i;
    for(i=0;i<size;i++) {
      u08 data = 0;
      status = fill_tx_func(&data);
      if(status!=PLIP_STATUS_OK) {
        break;
      }
      status = set_next_byte(data, toggle_expect, PLIP_STATE_DATA);
      if(status!=PLIP_STATUS_OK) {
        break;
      }
      toggle_expect = !toggle_expect;
    }
    // wait for last toggle
    if(status == PLIP_STATUS_OK) {
      status = wait_line_toggle(toggle_expect, PLIP_STATE_LAST_DATA);
    }
    // update real size
    pkt->real_size = i;
  }
  
  // restore: data input
  par_low_data_set_input();

  // reset strobe count
  par_low_strobe_count = 0;

  // reset HS_REQUEST line
  CLR_REQ();
  
  // wait for output state to end -> SELECT=0
  if(status == PLIP_STATUS_OK) {
    status = wait_for_select(0, PLIP_STATE_END);
  }
  
  return status;
}
