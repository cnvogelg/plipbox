#include "types.h"

#ifdef DEBUG_MODE
#define DEBUG
#endif

#include "debug.h"
#include "mode_mod.h"
#include "mode.h"
#include "mode_nic.h"
#include "pkt_buf.h"
#include "nic.h"
#include "param.h"
#include "proto_cmd_shared.h"

static u08 rx_res;

static u08 attach(void)
{
  u08 res = nic_attach_params();
  if(res != NIC_OK) {
    return MODE_ERROR;
  }

  return MODE_OK;
}

static void detach(void)
{
  nic_detach();
}

static void ping(void)
{
  nic_ping();
}

static u08 poll_status(void)
{
  u08 status = 0;

  if(nic_rx_num_pending() > 0) {
    status = PROTO_CMD_STATUS_RX_PENDING;
  }

  // poll link status if its available
  if(nic_has_link_status()) {
    u08 link_status = 0;
    u08 ok = nic_ioctl(NIC_IOCTL_GET_LINK_STATUS, &link_status);
    if(ok == NIC_OK) {
      if(link_status) {
        status |= PROTO_CMD_STATUS_LINK_UP;
      }
    } else {
      DT; DS("nic link status err:"); DB(ok); DNL;
    }
  }
  // fake link status
  else {
    status |= PROTO_CMD_STATUS_LINK_UP;
  }

  //DS("poll_status:"); DB(status); DNL;
  return status;
}

static u08 *tx_begin(u16 size)
{
  if(nic_is_direct()) {
    nic_tx_direct_begin(size);
    return NULL;
  } else {
    return pkt_buf;
  }
}

static u08 tx_end(u16 size)
{
  u08 res;

  if(nic_is_direct()) {
    res = nic_tx_direct_end(size);
  } else {
    res = nic_tx_data(pkt_buf, size);
  }

  if(res != NIC_OK) {
    return PROTO_CMD_STATUS_TX_ERROR;
  }
  return 0;
}

static u08 rx_size(u16 *got_size)
{
  u08 res = nic_rx_size(got_size);
  if(res != NIC_OK) {
    return MODE_ERROR;
  }
  return MODE_OK;
}

static u08 *rx_begin(u16 size)
{
  if(nic_is_direct()) {
    nic_rx_direct_begin(size);
    return NULL;
  } else {
    rx_res = nic_rx_data(pkt_buf, size);
    return pkt_buf;
  }
}

static u08 rx_end(u16 size)
{
  u08 res;

  if(nic_is_direct()) {
    res = nic_rx_direct_end(size);
  } else {
    res = rx_res;
  }

  if(res != NIC_OK) {
    return PROTO_CMD_STATUS_RX_ERROR;
  }
  return 0;
}

// define module
static const char ROM_ATTR mod_name[] = "nic";
const mode_mod_t ROM_ATTR mode_mod_nic = {
  .name = mod_name,

  .attach = attach,
  .detach = detach,

  .ping = ping,
  .poll_status = poll_status,

  .tx_begin = tx_begin,
  .tx_end = tx_end,

  .rx_size = rx_size,
  .rx_begin = rx_begin,
  .rx_end = rx_end
};
