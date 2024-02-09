#include "types.h"

#ifdef DEBUG_MODE
#define DEBUG
#endif

#include "debug.h"
#include "nic.h"
#include "nic_mod.h"
#include "nic_enc28j60.h"
#include "pkt_buf.h"

static u08 attach(u08 flags, mac_t mac)
{
  pkt_size = 0;
  return 0;
}

static void detach(void)
{

}

static void enable(void)
{

}

static void disable(void)
{

}

static u08 rx_num_pending(void)
{
  return (pkt_size > 0) ? 1 : 0;
}

static u16 rx_size(void)
{
  return pkt_size;
}

static u08 rx_data(u08 *buf, u16 size)
{
  pkt_size = 0;
  return 0;
}

static u08 tx_data(const u08 *buf, u16 size)
{
  pkt_size = size;
  return 0;
}

static u08 ioctl(u08 ioctl, u08 *value)
{
  return 0;
}

// ----- NIC module -----
static const char ROM_ATTR name[] = "loop";
const nic_mod_t ROM_ATTR nic_mod_loop = {
  .name = name,
  .capabilities = 0,

  .attach = attach,
  .detach = detach,

  .enable = enable,
  .disable = disable,

  .rx_num_pending = rx_num_pending,
  .rx_size = rx_size,
  .rx_data = rx_data,
  .tx_data = tx_data,

  .ioctl = ioctl
};
