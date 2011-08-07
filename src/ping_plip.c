#include "ping_plip.h"
#include "pkt_buf.h"
#include "board.h"
#include "ser_parse.h"
#include "pkt_buf.h"
#include "plip.h"
#include "uart.h"
#include "uartutil.h"
#include "ip.h"

static u08 pos;

static u08 begin_rx(plip_packet_t *pkt)
{
  pos = 0;
  return PLIP_STATUS_OK;
}

static u08 fill_rx(u08 *data)
{
  if(pos < PKT_BUF_SIZE) {
    pkt_buf[pos++] = *data;
  }
  return PLIP_STATUS_OK;
}

static u08 fill_tx(u08 *data)
{
  if(pos < PKT_BUF_SIZE) {
    *data = pkt_buf[pos++];
  }
  return PLIP_STATUS_OK;
}

static u08 end_rx(plip_packet_t *pkt)
{
  return PLIP_STATUS_OK;
}

void ping_plip_init(void)
{
  plip_recv_init(begin_rx, fill_rx, end_rx);
  plip_send_init(fill_tx);
  ser_parse_init(0,0); // use serial echo
}

void ping_plip_loop(void)
{  
  led_green_on(); 
  while(1) {
    ser_parse_worker();
    
    u08 status = plip_recv(&pkt);
    if(status != PLIP_STATUS_IDLE) {
      led_green_off();

      if(status == PLIP_STATUS_OK) {
        // is a ping packet?
        if(ip_icmp_is_ping_request(pkt_buf)) {
          // make reply
          ip_icmp_ping_request_to_reply(pkt_buf);

          // send reply
          pos = 0;
          status = plip_send(&pkt);
          if(status == PLIP_STATUS_CANT_SEND) {
            uart_send('C');
          } else if(status != PLIP_STATUS_OK) {
            uart_send('T');
            uart_send_hex_byte_crlf(status);
            uart_send_hex_word_crlf(pkt.real_size);
            uart_send_hex_word_crlf(pkt.size);
          } else {
            uart_send('.');
          }

        } else {
          // no ICMP request
          uart_send('?');
          uart_send_crlf();
        }
      } else {
        // show plip_recv error
        uart_send('R');
        uart_send_hex_byte_crlf(status);
        uart_send_hex_word_crlf(pkt.real_size);
        uart_send_hex_word_crlf(pkt.size);
      }

      led_green_on();
    }
  }
}

