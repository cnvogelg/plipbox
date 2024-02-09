#include "types.h"

#ifdef DEBUG_MODE
#define DEBUG
#endif

#include "debug.h"
#include "nic.h"
#include "nic_mod.h"
#include "nic_enc28j60.h"
#include "enc28j60.h"
#include "pkt_buf.h"

static u08 attach(u08 flags, mac_t mac)
{
  u08 res = enc28j60_reset(0);
  if(res == ENC28J60_ERROR_NOT_FOUND) {
    return NIC_ERROR_NOT_FOUND;
  }
  pkt_size = 0;
  enc28j60_setup_buffers();
  return NIC_OK;
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
  enc28j60_rx_begin_loop_back();
  enc28j60_rx_data(buf, size);
  enc28j60_rx_end_loop_back();
  pkt_size = 0;
  return 0;
}

static u08 tx_data(const u08 *buf, u16 size)
{
  enc28j60_tx_begin_loop_back();
  enc28j60_tx_data(buf, size);
  enc28j60_tx_end_loop_back();
  pkt_size = size;
  return 0;
}

static void rx_direct_begin(u16 size)
{
  enc28j60_rx_begin_loop_back();
}

static u08 rx_direct_end(u16 size)
{
  enc28j60_rx_end_loop_back();
  return 0;
}

static void tx_direct_begin(u16 size)
{
  enc28j60_tx_begin_loop_back();
}

static u08 tx_direct_end(u16 size)
{
  enc28j60_tx_end_loop_back();
  return 0;
}

static u08 ioctl(u08 cmd, u08 *value)
{
  switch(cmd) {
  case NIC_IOCTL_GET_HW_VERSION:
    *value = enc28j60_hw_revision();
    return NIC_OK;
  case NIC_IOCTL_GET_LINK_STATUS:
    *value = enc28j60_link_up();
    return NIC_OK;
  default:
    return NIC_ERROR_NOT_FOUND;
  }
}

// ----- NIC module -----
static const char ROM_ATTR name[] = "loop";
const nic_mod_t ROM_ATTR nic_mod_enc28j60 = {
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

  .rx_direct_begin = rx_direct_begin,
  .rx_direct_end = rx_direct_end,
  .tx_direct_begin = tx_direct_begin,
  .tx_direct_end = tx_direct_end,

  .ioctl = ioctl
};
