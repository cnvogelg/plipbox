#include "ping_slip.h"
#include "slip.h"
#include "stats.h"
#include "board.h"
#include "pkt_buf.h"
#include "uart.h"
#include "ser_parse.h"
#include "ip.h"

static u16 pos;
static u16 size;

static void slip_data(u08 data)
{
  if(pos < PKT_BUF_SIZE) {
    pkt_buf[pos] = data;
    pos++;
  }
}

static void slip_end(void)
{
  size = pos;
  pos = 0;
}

void ping_slip_init(void)
{
  slip_push_init(slip_data, slip_end);
  ser_parse_init(slip_push,0);
  pos = 0;
}

void ping_slip_loop(void)
{  
  led_green_on(); 
  while(1) {
    // receive from serial and trigger slip_push
    ser_parse_worker();
    
    // do we receive a slip end?
    if(size > 0) {
      // stop receiption
      uart_stop_reception();
      led_green_off();
      
      if(ip_icmp_is_ping_request(pkt_buf)) {
        // make reply
        ip_icmp_ping_request_to_reply(pkt_buf);

        // send reply
        slip_send_end();
        for(u16 i=0;i<size;i++) {
          slip_send(pkt_buf[i]);
        }
        slip_send_end();
      }
      
      // start receiption again
      led_green_on();
      uart_start_reception();
      size = 0;
    }
  }
}

