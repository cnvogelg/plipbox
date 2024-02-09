#include "types.h"

#ifdef DEBUG_LOOP_BUF
#define DEBUG
#endif

#include "debug.h"
#include "mode_mod.h"
#include "mode.h"
#include "mode_nic.h"
#include "pkt_buf.h"
#include "nic.h"
#include "param.h"

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

static u08 rx_poll(void)
{
  if(nic_rx_num_pending() > 0) {
    return MODE_RX_PENDING;
  }
  return MODE_OK;
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
    return MODE_ERROR;
  }
  return MODE_OK;
}

static u16 rx_size(void)
{
  return nic_rx_size();
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
    return MODE_ERROR;
  }
  return MODE_OK;
}

// define module
static const char ROM_ATTR mod_name[] = "nic";
const mode_mod_t ROM_ATTR mode_mod_nic = {
  .name = mod_name,

  .attach = attach,
  .detach = detach,

  .rx_poll = rx_poll,

  .tx_begin = tx_begin,
  .tx_end = tx_end,

  .rx_size = rx_size,
  .rx_begin = rx_begin,
  .rx_end = rx_end
};
