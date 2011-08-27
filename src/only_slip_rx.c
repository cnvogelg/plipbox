#include "only_slip_rx.h"

#include "param.h"
#include "ser_parse.h"
#include "slip_rx.h"
#include "board.h"

void only_slip_rx_loop(void)
{
  slip_rx_init();
  
  led_green_on(); 
  while(param.mode == PARAM_MODE_ONLY_SLIP_RX) {
    ser_parse_worker();
    slip_rx_worker();
  }
  led_green_off();
}
