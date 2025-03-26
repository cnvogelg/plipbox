#include "types.h"

#ifdef DEBUG_MODE
#define DEBUG
#endif

#include "debug.h"
#include "mode_mod.h"
#include "mode.h"
#include "pkt_buf.h"
#include "proto_error_shared.h"
#include "proto_api.h"
#include "nic.h"

static u16 loop_size;

static u08 attach(void)
{
  loop_size = 0;

  // fake link up
  proto_api_set_link_status(NIC_LINK_STATUS_UP);

  return MODE_STATUS_ATTACHED;
}

static void detach(void)
{
  proto_api_set_link_status(NIC_LINK_STATUS_DOWN);
}

static void ping(void)
{
}

static void work(void)
{
}

static u08 *tx_begin(u16 size)
{
  return pkt_buf;
}

static u16 tx_end(u16 size)
{
  loop_size = size;
  proto_api_set_rx_pending();
  return PROTO_ERROR_TX_OK;
}

static u16 rx_size(void)
{
  u16 result = loop_size;
  loop_size = 0;
  return result;
}

static u08 *rx_begin(u16 size)
{
  return pkt_buf;
}

static u16 rx_end(u16 size)
{
  return PROTO_ERROR_RX_OK;
}

// define module
static const char ROM_ATTR mod_name[] = "loop_buf";
const mode_mod_t ROM_ATTR mode_mod_loop_buf = {
  .name = mod_name,
  .tag = MODE_TAG_LOOP,

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
