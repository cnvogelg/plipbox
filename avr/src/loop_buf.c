#include "types.h"

#ifdef DEBUG_LOOP_BUF
#define DEBUG
#endif

#include "debug.h"
#include "mode_mod.h"
#include "mode.h"
#include "pkt_buf.h"

static u16 loop_size;

static u08 attach(void)
{
  loop_size = 0;
  return MODE_OK;
}

static void detach(void)
{
}

static u08 rx_poll(void)
{
  if(loop_size > 0) {
    return MODE_RX_PENDING;
  }
  return MODE_OK;
}

static u08 *tx_begin(u16 size)
{
  return pkt_buf;
}

static u08 tx_end(u16 size)
{
  loop_size = size;
  return MODE_OK;
}

static u16 rx_size(void)
{
  u16 size = loop_size;
  loop_size = 0;
  return size;
}

static u08 *rx_begin(u16 size)
{
  return pkt_buf;
}

static u08 rx_end(u16 size)
{
  return MODE_OK;
}

// define module
static const char PROGMEM mod_name[] = "loop_buf";
const mode_mod_t PROGMEM mode_mod_loop_buf = {
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
