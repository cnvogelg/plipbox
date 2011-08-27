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
  
  led_green_on(); 
  while(param.mode == PARAM_MODE_ONLY_PLIP_RX) {
    ser_parse_worker();
    
    // receive PLIP and send to SLIP
    u08 status = plip_recv(&pkt);
    if(status != PLIP_STATUS_IDLE) {
      led_green_off();
      
      if(status == PLIP_STATUS_OK) {
        bench_submit(pkt.size);
        stats.rx_cnt++;
        stats.rx_bytes+=pkt.size;
      } else {
        error ++;
        stats.rx_err++;
        stats.last_rx_err=status;
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

      led_green_on();
    }
  }
  led_green_off();
  
  bench_end();
}
