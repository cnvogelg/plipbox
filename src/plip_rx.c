#include "plip_rx.h"
#include "slip.h"
#include "board.h"
#include "stats.h"
#include "pkt_buf.h"

static u08 begin_rx(plip_packet_t *pkt)
{
  slip_send_end();
  return PLIP_STATUS_OK;
}

static u08 fill_rx(u08 *data)
{
  slip_send(*data);
  return PLIP_STATUS_OK;
}

static u08 end_rx(plip_packet_t *pkt)
{
  slip_send_end();
  return PLIP_STATUS_OK;
}

void plip_rx_init(void)
{
  plip_recv_init(begin_rx, fill_rx, end_rx);
}

u08 plip_rx_worker(void)
{
    // do we have a PLIP packet waiting?
    if(plip_can_recv() == PLIP_STATUS_OK) {
      led_green_off();
      
      u08 status = plip_recv(&pkt);
      // packet error
      if(status != PLIP_STATUS_OK) {
        stats.last_rx_err = status;
        stats.rx_err++;
      } 
      // packet ok
      else {
        stats.rx_cnt++;
        stats.rx_bytes+=pkt.size;
      }
      
      led_green_on();
      return status;
    } 
    // nothing to do - idle
    else {
      return PLIP_STATUS_IDLE;
    }
}
