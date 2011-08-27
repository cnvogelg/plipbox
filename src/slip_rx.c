#include "slip_rx.h"
#include "uart.h"
#include "slip.h"
#include "plip.h"
#include "board.h"
#include "pkt_buf.h"
#include "log.h"
#include "ser_parse.h"
#include "uartutil.h"
#include "param.h"
#include "stats.h"

static u16 pkt_buf_pos = 0;
static u16 send_pos = 0;
static u16 ip_size = 0;

static u08 skip_packet = 0;
static u08 transmit_packet = 0;
static u08 dropped_packet = 0;

static plip_packet_t rpkt = {
  .crc_type = PLIP_NOCRC,
  .size = 0,
  .crc = 0,
  .type = 0x800
};

typedef u08 (*send_func_t)(plip_packet_t *);
static send_func_t send_func;

// callback to retrieve next byte for plip packet send
static u08 get_plip_data(u08 *data)
{
  // fetch data from buffer
  if(send_pos < pkt_buf_pos) {
    *data = pkt_buf[send_pos++];
    return PLIP_STATUS_OK;
  } 
  // fetch data directly from serial
  else {
    u08 ok = slip_read(data);
    
    // store in buf for potential retry
    if(pkt_buf_pos < PKT_BUF_SIZE) {
      pkt_buf[pkt_buf_pos++] = *data;
      send_pos ++;
    }
    
    if(ok == SLIP_STATUS_OK) {
      return PLIP_STATUS_OK;
    } else {
      return PLIP_STATUS_CALLBACK_FAILED;
    }
  }
}

// fake the plip send by only retrieving the data
static u08 fake_send(plip_packet_t *pkt)
{
  u08 status = PLIP_STATUS_OK;
  u16 size = pkt->size;
  for(u16 i=0;i<size;i++) {
    u08 d;
    status = get_plip_data(&d);
    if(status != PLIP_STATUS_OK) {
      return status;
    }
  }
  return status;
}

// a SLIP data was received
static void slip_rx_data(u08 data)
{
  // currently skipping packet?
  if(skip_packet) {
    return;
  }
  
  if(pkt_buf_pos == 0) {
    // check for IPv4 packet begin
    if((data & 0xf0) == 0x40) {
      pkt_buf[0] = data;
      pkt_buf_pos = 1;
    } else {
      // hmm? skip packet!
      skip_packet = 1;
      dropped_packet = 1;
    }
  }
  // store header bytes
  else if(pkt_buf_pos < PKT_BUF_SIZE) {
    pkt_buf[pkt_buf_pos++] = data;
  }
  // argh! no storage left -> need to drop packet
  else {
    dropped_packet = 1;
    transmit_packet = 0;
    skip_packet = 1;
  }
  
  // first four header bytes received, i.e. we know the size of the packet 
  // -> worker shall transmit packet
  if(pkt_buf_pos == 4) {
    transmit_packet = 1;
    
    // calc size
    ip_size = (u16)pkt_buf[2] << 8 | (u16)pkt_buf[3];
  }
}

// a SLIP END was received
static void slip_rx_end(void)
{
  // reset parse state
  pkt_buf_pos = 0;
  skip_packet = 0;
  transmit_packet = 0;
  dropped_packet = 0;
}

// ----- API: init -----
void slip_rx_init(void)
{
  // setup plip
  plip_send_init(get_plip_data);

  // setup serial + slip
  slip_push_init(slip_rx_data, slip_rx_end);
  ser_parse_set_data_func(slip_push);
  
  // check fake_tx
  if(param.fake_tx) {
    send_func = fake_send;
  } else {
    send_func = plip_send;
  }
}

// ----- API: worker -----
u08 slip_rx_worker(void)
{
  u08 result = SLIP_STATUS_IDLE;

  // a transmit was signalled by last data call
  // (we got the ip header and its length)
  if(transmit_packet) {
    led_yellow_on();
    
    rpkt.size = ip_size;
    
    // send packet via plip (or fake send)
    send_pos = 0;
    u08 status = send_func(&rpkt);

    // a receive from plip is pending -> do this first
    if(status == PLIP_STATUS_CANT_SEND) {
      return SLIP_STATUS_ABORT;
    }

    if(status != PLIP_STATUS_OK) {
      // some error occurred while sending
      stats.tx_err ++;
      stats.last_tx_err = status;
      result = SLIP_STATUS_ERROR;
    } else {
      // packet sent ok
      stats.tx_cnt ++;
      stats.tx_bytes += ip_size;
      result = SLIP_STATUS_OK;
    }
      
    // skip remainder of packet
    skip_packet = 1;
    transmit_packet = 0;
    
    led_yellow_off();
  }
  
  // a drop occurred
  if(dropped_packet) {
    dropped_packet = 0;
    result = SLIP_STATUS_DROP;
    stats.tx_drop ++;
  }
  
  return result;
}
