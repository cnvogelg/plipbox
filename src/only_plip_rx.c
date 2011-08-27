#include "only_plip_rx.h"
#include "plip_rx.h"
#include "param.h"
#include "ser_parse.h"
#include "board.h"
#include "bench.h"
#include "pkt_buf.h"
#include "stats.h"

void only_plip_rx_loop(void)
{
  u16 count = 0;
  u16 error = 0;
  
  plip_rx_init();
  ser_parse_set_data_func(0); // use serial echo

  bench_begin();
  
  // ----- main loop -----
  led_green_on(); 
  while(param.mode == PARAM_MODE_ONLY_PLIP_RX) {
    // serial handling
    ser_parse_worker();
    
    // receive PLIP
    u08 status = plip_rx_worker();    
    if(status != PLIP_STATUS_IDLE) {
      
      if(status == PLIP_STATUS_OK) {
        bench_submit(pkt.size);
      } else {
        error ++;
      }
      
      // give summary
      count ++;
      if(count == 256) {
        count = 0;
        if(error>0) {
          led_red_on();
          error = 0;
        } else {
          led_red_off();
        }        
        bench_end();
        bench_begin();
      }
    }
  }
  led_green_off();
  
  bench_end();
}
