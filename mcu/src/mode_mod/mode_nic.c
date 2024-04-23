#include "types.h"

#ifdef DEBUG_MODE
#define DEBUG
#endif

#include "debug.h"
#include "mode_mod.h"
#include "mode.h"
#include "mode_nic.h"
#include "mode_cmd.h"
#include "pkt_buf.h"
#include "nic.h"
#include "param.h"
#include "proto_error_shared.h"

static u08 signalled_rx_pending = 0;

static u08 attach(void)
{
  u08 res = nic_attach_params();
  if(res != NIC_OK) {
    return MODE_ERROR;
  }

  // fake link status
  if(!nic_has_link_status()) {
    mode_cmd_set_link_status(PROTO_STATUS_LINK_UP);
  }

  signalled_rx_pending = 0;

  return MODE_OK;
}

static void detach(void)
{
  mode_cmd_set_link_status(PROTO_STATUS_LINK_DOWN);

  nic_detach();
}

static void ping(void)
{
  nic_ping();
}

static void work(void)
{
  // set rx pending
  if(!signalled_rx_pending) {
    if(nic_rx_num_pending() > 0) {
      mode_cmd_set_rx_pending();
      signalled_rx_pending = 1;
    }
  }

  // poll link status if its available
  if(nic_has_link_status()) {
    u16 link_status = 0;
    u08 ok = nic_ioctl(NIC_IOCTL_GET_LINK_STATUS, &link_status);
    if(ok == NIC_OK) {
      u16 current_status = mode_cmd_get_link_status();
      if(link_status != current_status) {
        mode_cmd_set_link_status(link_status);
      }
    } else {
      mode_cmd_set_link_status(PROTO_STATUS_LINK_UNKNOWN);
    }
  }
}

static u08 *tx_begin(u16 size)
{
  return nic_tx_begin(size);
}

static u16 tx_end(u16 size)
{
  u08 res = nic_tx_end(size);
  if(res != NIC_OK) {
    return PROTO_ERROR_TX_FAILED;
  }
  return PROTO_ERROR_TX_OK;
}

static u16 rx_size(void)
{
  u16 got_size = 0;
  u08 res = nic_rx_size(&got_size);
  if(res != NIC_OK) {
    return 0;
  }
  return got_size;
}

static u08 *rx_begin(u16 size)
{
  return nic_rx_begin(size);
}

static u16 rx_end(u16 size)
{
  signalled_rx_pending = 0;

  u08 res = nic_rx_end(size);
  if(res != NIC_OK) {
    return PROTO_ERROR_RX_FAILED;
  }
  return PROTO_ERROR_RX_OK;
}

// define module
static const char ROM_ATTR mod_name[] = "nic";
const mode_mod_t ROM_ATTR mode_mod_nic = {
  .name = mod_name,
  .tag = MODE_TAG_NIC,

  .attach = attach,
  .detach = detach,

  .ping = ping,
  .work = work,

  .tx_begin = tx_begin,
  .tx_end = tx_end,

  .rx_size = rx_size,
  .rx_begin = rx_begin,
  .rx_end = rx_end
};
