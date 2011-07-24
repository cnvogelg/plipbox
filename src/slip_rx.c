#include "slip_rx.h"
#include "uart.h"
#include "slip.h"
#include "plip.h"
#include "board.h"

#include "uartutil.h"

#define IP_HDR_LEN  64

static u08 ip_buf[IP_HDR_LEN];
static u08 ip_buf_pos = 0;
static u08 skip_packet = 0;
static u08 transmit_packet = 0;
static u08 dropped_packet = 0;
static u16 ip_size = 0;

static u08 send_pos = 0;

static plip_packet_t pkt = {
  .crc_type = PLIP_NOCRC,
  .size = 0,
  .crc = 0,
  .type = 0x800
};

static u08 get_plip_data(u08 *data)
{
  // fetch data from buffer
  if(send_pos < ip_buf_pos) {
    *data = ip_buf[send_pos++];
    return PLIP_STATUS_OK;
  } 
  // fetch data directly from serial
  else {
    u08 ok = slip_read(data);
    if(ok == SLIP_STATUS_OK) {
      return PLIP_STATUS_OK;
    } else {
      return PLIP_STATUS_CALLBACK_FAILED;
    }
  }
}

u08 fake_send(plip_packet_t *pkt)
{
  u08 status = PLIP_STATUS_OK;
  u16 size = pkt->size;
  //slip_send_end();
  for(u16 i=0;i<size;i++) {
    u08 d;
    status = get_plip_data(&d);
    if(status != PLIP_STATUS_OK) {
      return status;
    }
    //slip_send(d);
  }
  //slip_send_end();
  return status;
}

void slip_rx_init(void)
{
  plip_send_init(get_plip_data);
}

//#deifne DEBUG_SLIP_RX
#ifdef DEBUG_SLIP_RX
#define DBG(x)      slip_send(x)
#define DBG_TAG()   slip_send_end()
#define DBG_BYTE(x) uart_send_hex_byte_crlf(x)
#define DBG_WORD(x) uart_send_hex_word_crlf(x)
#else
#define DBG(x)
#define DBG_TAG()
#define DBG_BYTE(x)
#define DBG_WORD(x)
#endif

void slip_rx_data(u08 data)
{
  // currently skipping packet?
  if(skip_packet) {
    DBG('-');
    return;
  }
  
  if(ip_buf_pos == 0) {
    // check for IPv4 packet begin
    if((data & 0xf0) == 0x40) {
      ip_buf[0] = data;
      ip_buf_pos = 1;
      DBG('b');
    } else {
      // hmm? skip packet!
      skip_packet = 1;
      dropped_packet = 1;
      DBG('B');
    }
  }
  // store header bytes
  else if(ip_buf_pos < IP_HDR_LEN) {
    DBG('x');
    ip_buf[ip_buf_pos++] = data;
  }
  // argh! no storage left -> need to drop packet
  else {
    dropped_packet = 1;
    transmit_packet = 0;
    skip_packet = 1;
    DBG('O');
  }
  
  // first four header bytes received, i.e. we know the size of the packet 
  // -> worker shall transmit packet
  if(ip_buf_pos == 4) {
    transmit_packet = 1;
    DBG('g');
    
    // calc size
    ip_size = (u16)ip_buf[2] << 8 | (u16)ip_buf[3];

    // stop receiption until worker can transmit
    //uart_stop_reception();
    led_yellow_on();    
  }
}

void slip_rx_end(void)
{
  DBG('.');
  DBG_TAG();
  
  // reset parse state
  ip_buf_pos = 0;
  skip_packet = 0;
  transmit_packet = 0;
  dropped_packet = 0;
}

u08 slip_rx_worker(void)
{
  u08 result = SLIP_STATUS_OK;

  // a transmit was signalled
  if(transmit_packet) {
    if(plip_is_send_allowed()) {
      DBG('T');
      //uart_start_reception();

      pkt.size = ip_size;
      
      // send packet via plip
      send_pos = 0;
#ifdef FAKE_SEND
      u08 status = fake_send(&pkt);
#else
      u08 status = plip_send(&pkt);
#endif
      if(status != PLIP_STATUS_OK) {
        DBG('E');
        result = SLIP_STATUS_ERROR;
      } else {
        DBG('e');
      }
      DBG_WORD(ip_size);
            
      // skip remainder of packet
      skip_packet = 1;
      transmit_packet = 0;
      led_yellow_off();
    }
  }
  
  // a drop occurred
  if(dropped_packet) {
    dropped_packet = 0;
    result = SLIP_STATUS_ERROR;
  }
  
  return result;
}
