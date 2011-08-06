#include "transfer.h"

#include "board.h"
#include "plip.h"
#include "slip.h"
#include "slip_rx.h"
#include "uart.h"
#include "uartutil.h"
#include "ser_parse.h"

static u08 begin_end_rx_to_slip(plip_packet_t *pkt)
{
  if(slip_send_end()) {
    return PLIP_STATUS_OK;
  } else {
    return PLIP_STATUS_CALLBACK_FAILED;
  }
}

static u08 fill_rx_to_slip(u08 *data)
{
  u08 d = *data;
  if(slip_send(d)) {
    return PLIP_STATUS_OK;
  } else {
    return PLIP_STATUS_CALLBACK_FAILED;
  }
}

static plip_packet_t pkt;


void transfer_init(void)
{
  plip_recv_init(begin_end_rx_to_slip, fill_rx_to_slip, begin_end_rx_to_slip);
  slip_rx_init();
}

void transfer_worker(void)
{
    // light green READY and enter main loop
    led_green_on();
    u16 error_count = 0;
    u08 rx_skip = 0;

  #ifdef WAIT_FOR_ONE
    u16 plip_rx_retries = 10;
    u08 got_one = 0;
  #endif  

    while(1) {

  #ifdef WAIT_FOR_ONE    
      // retry recv?
      timer_100us = 0;
      got_one = 0;
      do {
  #endif
        // incoming packet?
        led_green_off();
        uart_stop_reception();

        // receive packet -> transfer to slip
        u08 status = plip_recv(&pkt);
        rx_skip = 0;
        if(status > PLIP_STATUS_IDLE) {
          error_count = 0x2fff;

          // send error packet
          slip_send_end();
          slip_send('Y');
          uart_send_hex_byte_crlf(status);
          slip_send_end();
        }

        uart_start_reception();
        led_green_on();
  #ifdef WAIT_FOR_ONE
         got_one = 1;
      }
      while(!got_one && (timer_100us < plip_rx_retries));
  #endif

      // trigger slip worker
      u08 slip_status = slip_rx_worker();
      if((slip_status == SLIP_RX_RESULT_DROP)||(slip_status == SLIP_RX_RESULT_TX_FAIL)) {
        error_count = 0x2fff;
      }
      else if(slip_status == SLIP_RX_RESULT_PLIP_RX_BEGUN_SKIP) {
        rx_skip = 1;
      }

      // update error LED
      if(error_count > 1) {
        led_red_on();
        error_count--;
      } else if(error_count == 1) {
        led_red_off();
      }

      // trigger serial state parser
      ser_parse_worker();
    }

}
