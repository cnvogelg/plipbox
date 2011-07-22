#include "plip.h"
#include "par_low.h"
#include "timer.h"

// recv funcs
static plip_packet_func   begin_rx_func = 0;
static plip_data_func     fill_rx_func = 0;
static plip_packet_func   end_rx_func = 0;

// send funcs
static plip_data_func     fill_tx_func = 0;

u16 plip_timeout = 5000; // in 100us

void plip_recv_init(plip_packet_func begin_func, 
                    plip_data_func   fill_func,
                    plip_packet_func end_func)
{
  begin_rx_func = begin_func;
  fill_rx_func  = fill_func;
  end_rx_func   = end_func;
}

void plip_send_init(plip_data_func   fill_func)
{
  fill_tx_func  = fill_func;
}

u08 plip_is_recv_begin(void)
{
  if(par_low_strobe_flag) {
    par_low_strobe_flag = 0;
    return 1;
  } else {
    return 0;
  }
}

u08 plip_is_send_allowed(void)
{
  // HS_LINE must be low
  return !par_low_get_pout() && !par_low_strobe_flag;
}

static u08 wait_line_toggle(u08 toggle_expect)
{
  // wait for new toggle value
  timer_100us = 0;
  while(timer_100us < plip_timeout) {
    u08 pout = par_low_get_pout();
    if((toggle_expect && pout) || (!toggle_expect && !pout)) {
      return PLIP_STATUS_OK;
    }
  }
  return PLIP_STATUS_TIMEOUT;
}

// ---------- Receive ----------

static u08 get_next_byte(u08 *data, u08 toggle_expect)
{
  // wait for HS_LINE toggle
  u08 status = wait_line_toggle(toggle_expect);
  if(status == PLIP_STATUS_OK) {
    // read byte
    *data = par_low_data_in();
    
    // toggle HS_REQUEST (BUSY)
    par_low_toggle_busy();
  }
  return status;
}

static u08 get_next_word(u16 *data, u08 toggle_expect)
{
  u08 a,b;
  u08 status = get_next_byte(&a, toggle_expect);
  if(status) {
    return status;
  }
  status = get_next_byte(&b, !toggle_expect);
  if(status) {
    return status;
  }
  *data = ((u16)a) << 8 | b;
  return PLIP_STATUS_OK;
}

static u08 get_next_dword(u32 *data, u08 toggle_expect)
{
  u16 a,b;
  u08 status = get_next_word(&a, toggle_expect);
  if(status != PLIP_STATUS_OK) {
    return status;
  }
  status = get_next_word(&b, toggle_expect);
  if(status != PLIP_STATUS_OK) {
    return status;
  }
  *data = ((u32)a) << 16 | b;
  return PLIP_STATUS_OK;
}

u08 plip_recv(plip_packet_t *pkt)
{
  // make sure my HS_LINE (POUT) is high
  if(!par_low_get_pout()) {
    return PLIP_STATUS_INVALID_START;
  }
  
  // first byte must be magic (0x42) 
  // this byte was set by last strobe received before calling this func
  u08 magic = 0;
  u08 status = get_next_byte(&magic, 1);
  if(status == PLIP_STATUS_OK) {
    if(magic != PLIP_MAGIC) {
      status = PLIP_STATUS_NO_MAGIC;
    }
  }
  
  // expect CRC type
  u08 crc_type = 0;
  if(status == PLIP_STATUS_OK) {
    status = get_next_byte(&crc_type, 0);
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
    status = get_next_word(&size, 1);
  }
  
  // read crc
  u16 crc;
  if(status == PLIP_STATUS_OK) {
    pkt->size = size - 6;
    status = get_next_word(&crc, 1);
  }
  size -= 2;
  
  // read type
  u32 type = 0;
  if(status == PLIP_STATUS_OK) {
    pkt->crc = crc;
    status = get_next_dword(&type, 1);
  }
  size -= 4;
  
  // report begin of frame
  if(status == PLIP_STATUS_OK) {
    pkt->type = type;
    status = begin_rx_func(pkt);
  }
  
  // read data
  if(status == PLIP_STATUS_OK) {
    u08 expect_toggle = 1;
    for(u16 i=0;i<size;i++) {
      u08 b;
      status = get_next_byte(&b, expect_toggle);
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
  }

  // report end of frame
  if(status == PLIP_STATUS_OK) {
    status = end_rx_func(pkt);
  }

  // clear all strobes occurred during recv
  par_low_strobe_flag = 0;

  // clear HS_REQUEST (BUSY) to signal end of transmission
  par_low_set_busy_lo();
  
  return status;
}

// ---------- Send ----------

static u08 set_next_byte(u08 data, u08 toggle_expect)
{
  // wait for new toggle value
  u08 status = wait_line_toggle(toggle_expect);
  if(status == PLIP_STATUS_OK) {
      // set new value
      par_low_data_out(data);
      
      // toggle BUSY
      par_low_toggle_busy();
  }
  return status;
}

static u08 set_next_word(u16 word, u08 toggle_expect)
{
  u08 a = (u08)(word >> 8);
  u08 status = set_next_byte(a, toggle_expect);
  if(status == PLIP_STATUS_OK) {
    u08 b = (u08)(word & 0xff);
    status = set_next_byte(b, !toggle_expect);
  }
  return status;
}

static u08 set_next_dword(u32 dword, u08 toggle_expect)
{
  u16 a = (u16)(dword >> 16);
  u08 status = set_next_word(a, toggle_expect);
  if(status == PLIP_STATUS_OK) {
    u16 b = (u16)(dword & 0xffff);
    status = set_next_word(b, toggle_expect);
  }
  return status;
}

u08 plip_send(plip_packet_t *pkt)
{
  u08 status = PLIP_STATUS_OK;
  
  // set my HS_REQUEST line
  par_low_set_busy_hi();
  
  // make sure my HS_LINE is low
  if(par_low_get_pout()) {
    // no. abort: clear myHS_REQUEST
    par_low_set_busy_lo();
    
    return PLIP_STATUS_INVALID_START;
  }
  
  // set data port to output
  par_low_data_set_output();
  
  // set magic on data bus and perform an ACK pulse to trigger receiption 
  par_low_data_out(PLIP_MAGIC);
  par_low_pulse_ack();
  
  // send crc type
  u08 crc_type = pkt->crc_type;
  status = set_next_byte(crc_type, 1);
  
  // send size
  if(status==PLIP_STATUS_OK) {
    u16 size = pkt->size + 6; // = + CRC (word) + TYPE (dword)
    status = set_next_word(size, 0);
  }
  
  // send crc
  if(status==PLIP_STATUS_OK) {
    u16 crc = pkt->crc;
    status = set_next_word(crc, 0);
  }
  
  // send type
  if(status==PLIP_STATUS_OK) {
    u32 type = pkt->type;
    status = set_next_dword(type, 0);
  }
  
  // send packet bytes
  if(status==PLIP_STATUS_OK) {
    u08 toggle_expect = 0;
    u16 size = pkt->size;
    for(u16 i=0;i<size;i++) {
      u08 data = 0;
      status = fill_tx_func(&data);
      if(status!=PLIP_STATUS_OK) {
        break;
      }
      status = set_next_byte(data, toggle_expect);
      if(status!=PLIP_STATUS_OK) {
        break;
      }
      toggle_expect = !toggle_expect;
    }
    // wait for last toggle
    if(status == PLIP_STATUS_OK) {
      status = wait_line_toggle(toggle_expect);
    }
  }
  
  // restore: data input
  par_low_data_set_input();

  // reset HS_REQUEST line
  par_low_set_busy_lo();
  
  return status;
}
