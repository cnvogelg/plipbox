#include "plip_rx.h"
#include "slip.h"

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
