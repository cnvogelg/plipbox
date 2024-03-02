#include "types.h"

#ifdef DEBUG_LOOP_BUF
#define DEBUG
#endif

#include "debug.h"
#include "mode_mod.h"
#include "mode.h"
#include "mode_loop_buf.h"
#include "pkt_buf.h"
#include "proto_cmd_shared.h"

static u16 loop_size;

static u08 attach(void)
{
  loop_size = 0;
  return MODE_OK;
}

static void detach(void)
{
}

static void ping(void)
{
}

static u08 poll_status(void)
{
  u08 status = PROTO_CMD_STATUS_LINK_UP;
  if(loop_size > 0) {
    status |= PROTO_CMD_STATUS_RX_PENDING;
  }
  return status;
}

static u08 *tx_begin(u16 size)
{
  return pkt_buf;
}

static u08 tx_end(u16 size)
{
  loop_size = size;
  return 0;
}

static u08 rx_size(u16 *got_size)
{
  *got_size = loop_size;
  loop_size = 0;
  return MODE_OK;
}

static u08 *rx_begin(u16 size)
{
  return pkt_buf;
}

static u08 rx_end(u16 size)
{
  return 0;
}

// define module
static const char ROM_ATTR mod_name[] = "loop_buf";
const mode_mod_t ROM_ATTR mode_mod_loop_buf = {
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
