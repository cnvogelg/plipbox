#include "plip.h"
#include "par_low.h"
#include "timer.h"

static plip_begin_rx_frame_func begin_rx_frame_func = 0;
static plip_fill_rx_frame_func fill_rx_frame_func = 0;

u16 plip_rx_timeout = 5000; // in 100us

void plip_init(plip_begin_rx_frame_func begin_func, plip_fill_rx_frame_func fill_func)
{
  begin_rx_frame_func = begin_func;
  fill_rx_frame_func = fill_func;
}

u08 plip_is_recv_begin(void)
{
  return par_low_get_select() && par_low_has_input();
}

static u08 get_next_byte(u08 *data)
{
  timer_100us = 0;
  while(timer_100us < plip_rx_timeout) {
    if(par_low_has_input()) {
      *data = par_low_get_input();
      return PLIP_STATUS_OK;
    }
  }
  return PLIP_STATUS_RX_TIMEOUT;
}

static u08 get_next_word(u16 *data)
{
  u08 a,b;
  u08 status = get_next_byte(&a);
  if(status) {
    return status;
  }
  status = get_next_byte(&b);
  if(status) {
    return status;
  }
  *data = ((u16)a) << 8 | b;
  return PLIP_STATUS_OK;
}

static u08 get_next_dword(u32 *data)
{
  u16 a,b;
  u08 status = get_next_word(&a);
  if(status) {
    return status;
  }
  status = get_next_word(&b);
  if(status) {
    return status;
  }
  *data = ((u32)a) << 16 | b;
  return PLIP_STATUS_OK;
}

u08 plip_recv(plip_packet_t *pkt)
{
  // first byte is magic (0x42)
  u08 magic = par_low_get_input();
  if(magic != PLIP_MAGIC) {
    return PLIP_STATUS_NO_MAGIC;
  }
  
  // expect CRC type
  u08 crc_type;
  u08 status = get_next_byte(&crc_type);
  if(status) {
    return status;
  }
  if((crc_type != PLIP_CRC) && (crc_type != PLIP_NOCRC)) {
    return PLIP_STATUS_INVALID_MAGIC;
  }
  
  // expect size
  u16 size;
  status = get_next_word(&size);
  if(status) {
    return status;
  }
  
  // read crc
  u16 crc;
  status = get_next_word(&crc);
  if(status) {
    return status;
  }
  size -= 2;
  
  // read type
  u32 type;
  status = get_next_dword(&type);
  if(status) {
    return status;
  }
  size -= 4;
  
  // setup packet
  pkt->crc_type = crc_type;
  pkt->size = size;
  pkt->crc = crc;
  pkt->type = type;
  
  // report begin frame
  status = begin_rx_frame_func(pkt);
  if(status) {
    return status;
  }
  
  // read data
  for(u16 i=0;i<size;i++) {
    u08 b;
    status = get_next_byte(&b);
    if(status) {
      return status;
    }
    
    // report to fill frame func
    status = fill_rx_frame_func(b);
    if(status) {
      return status;
    }
  }

  return PLIP_STATUS_OK;
}
