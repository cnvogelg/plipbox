#include "ping_plip.h"
#include "pkt_buf.h"
#include "board.h"
#include "ser_parse.h"
#include "pkt_buf.h"
#include "plip.h"
#include "uart.h"
#include "uartutil.h"
#include "ip.h"
#include "stats.h"
#include "param.h"

static u16 pos;

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

void ping_plip_loop(void)
{
  plip_recv_init(begin_rx, fill_rx, end_rx);
  plip_send_init(fill_tx);
  ser_parse_set_data_func(0); // use serial echo
  stats_reset();
  
  led_green_on(); 
  while(param.mode == PARAM_MODE_PING_PLIP) {
    ser_parse_worker();
    
    u08 status = plip_recv(&pkt);
    if(status != PLIP_STATUS_IDLE) {
      stats.pkt_count++;

      led_green_off();

      if(status == PLIP_STATUS_OK) {
        // is a ping packet?
        if(ip_icmp_is_ping_request(pkt_buf)) {
          stats.pkt_bytes += ip_hdr_get_size(pkt_buf);
          
          // make reply
          ip_icmp_ping_request_to_reply(pkt_buf);

          // send reply
          pos = 0;
          status = plip_send(&pkt);
          if(status == PLIP_STATUS_CANT_SEND) {
            stats.pkt_last_tx_err = status;
            stats.pkt_tx_err++;
            uart_send('C');
          } else if(status != PLIP_STATUS_OK) {
            stats.pkt_last_tx_err = status;
            stats.pkt_tx_err++;
            uart_send('T');
          }
          
        } else {
          // no ICMP request
          uart_send('?');
          uart_send_hex_byte_spc(pkt_buf[0]);
          uart_send_hex_byte_spc(pkt_buf[9]);
          uart_send_hex_byte_spc(pkt_buf[20]);
        }
      } else {
        stats.pkt_rx_err++;
        stats.pkt_last_rx_err = status;
        uart_send('R');
      }

      // give summary
      if(stats.pkt_count == 256) {
        u08 err = (stats.pkt_tx_err > 0) || (stats.pkt_rx_err > 0);
        if(err) {
          led_red_on();
        } else {
          led_red_off();
        }
        
        stats_dump();
        
      }

      led_green_on();
    }
  }
  led_green_off();
}

