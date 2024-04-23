/*
  mode_cmd.c - implement the proto_cmd_api for module operation
*/

#include "types.h"

#ifdef DEBUG_MODE
#define DEBUG
#endif

#include "debug.h"
#include "mode.h"
#include "mode_cmd.h"
#include "hw_uart.h"
#include "uartutil.h"
#include "proto_cmd.h"

#include "proto_event_shared.h"
#include "proto_status_shared.h"

static u16 event_mask;
static u08 trigger_update;
static u16 link_status;
static u16 hw_status;
static u16 req_event_mask;

static u16 rx_error;
static u16 rx_drop_count;
static u16 tx_error;
static u16 tx_drop_count;

void mode_cmd_init(void)
{
  event_mask = 0;
  trigger_update = 0;
  link_status = PROTO_STATUS_LINK_UNKNOWN;
  hw_status = PROTO_STATUS_HW_OFF;

  rx_error = 0;
  rx_drop_count = 0;
  tx_error = 0;
  tx_drop_count = 0;
  req_event_mask = 0;
}

u16 proto_cmd_api_get_version(void)
{
  return VERSION_MAJ << 8 | VERSION_MIN;
}

void proto_cmd_api_attach(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("proto: attach"));
  uart_send_crlf();

  mode_attach();
}

void proto_cmd_api_detach(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("proto: detach"));
  uart_send_crlf();

  mode_detach();
}

// event mask stuff
u16 proto_cmd_api_event_mask(void)
{
  // clear mask on read
  u16 msk = event_mask;
  DS("event_mask:"); DW(msk); DNL;
  event_mask = 0;
  return msk;
}

void mode_cmd_add_event_mask(u16 mask)
{
  mode_cmd_set_event_mask(mask | event_mask);
}

void mode_cmd_set_event_mask(u16 mask)
{
  // nothing to do
  if(mask == event_mask) {
    return;
  }

  event_mask = mask;
  trigger_update = 1;
  DS("set_event_mask:"); DW(event_mask); DNL;
}

u16 mode_cmd_take_event_mask(void)
{
  // take and clear the mask
  // any pending trigger is removed
  u16 msk = event_mask;
  event_mask = 0;
  trigger_update = 0;
  return msk;
}

void mode_cmd_process_trigger(void)
{
  // if not in a tx/rx phase then check mode status, e.g. rx packet or link up/down
  if(proto_cmd_get_state() == PROTO_CMD_STATE_IDLE) {
    if(trigger_update) {
      // send async trigger to host -> will check status
      proto_cmd_trigger_status();
      trigger_update = 0;
    }
  }
}

// link and hw status

void mode_cmd_set_rx_pending(void)
{
  mode_cmd_add_event_mask(PROTO_EVENT_RX_PENDING);
}

u16 proto_cmd_api_link_status(void)
{
  return link_status;
}

void mode_cmd_set_link_status(u16 status)
{
  link_status = status;
  DS("set_link_status:"); DW(status); DNL;
  mode_cmd_add_event_mask(PROTO_EVENT_LINK_STATUS);
}

u16 mode_cmd_get_link_status(void)
{
  return link_status;
}

u16 proto_cmd_api_hw_status(void)
{
  return hw_status;
}

void mode_cmd_set_hw_status(u16 status)
{
  hw_status = status;
  DS("set_hw_status:"); DW(status); DNL;
  mode_cmd_add_event_mask(PROTO_EVENT_HW_STATUS);
}

u16 proto_cmd_api_req_event_mask(void)
{
  return req_event_mask;
}

void mode_cmd_set_req_event_mask(u16 req_mask)
{
  if(req_event_mask == req_mask) {
    return;
  }

  req_event_mask = req_mask;
  mode_cmd_add_event_mask(PROTO_EVENT_REQ_EVENTS);
}

void mode_cmd_add_req_event_mask(u16 req_mask)
{
  mode_cmd_set_req_event_mask(req_event_mask | req_mask);
}

// rx status

u16 proto_cmd_api_rx_error(void)
{
  return rx_error;
}

void mode_cmd_set_rx_error(u16 error)
{
  rx_error = error;
  DS("rx_error:"); DW(rx_error); DNL;
  if(rx_error != 0) {
    mode_cmd_add_event_mask(PROTO_EVENT_RX_ERROR);
  }
}

u16 proto_cmd_api_rx_drop_count(void)
{
  return rx_drop_count;
}

void mode_cmd_set_rx_drop_count(u16 drops)
{
  rx_drop_count = drops;
  DS("rx_drop_count:"); DW(rx_drop_count); DNL;
  mode_cmd_add_event_mask(PROTO_EVENT_RX_DROPS);
}

void mode_cmd_add_rx_drop_count(u16 drops)
{
  mode_cmd_set_rx_drop_count(rx_drop_count + drops);
}

// tx status

u16 proto_cmd_api_tx_error(void)
{
  return tx_error;
}

void mode_cmd_set_tx_error(u16 error)
{
  tx_error = error;
  DS("tx_error:"); DW(tx_error); DNL;
  if(tx_error != 0) {
    mode_cmd_add_event_mask(PROTO_EVENT_TX_ERROR);
  }
}

u16 proto_cmd_api_tx_drop_count(void)
{
  return tx_drop_count;
}

void mode_cmd_set_tx_drop_count(u16 drops)
{
  tx_drop_count = drops;
  DS("tx_drop_count:"); DW(tx_drop_count); DNL;
  mode_cmd_add_event_mask(PROTO_EVENT_TX_DROPS);
}

void mode_cmd_add_tx_drop_count(u16 drops)
{
  mode_cmd_set_tx_drop_count(tx_drop_count + drops);
}

// ----- ping -----

void proto_cmd_api_ping(void)
{
  mode_ping();
}

// ----- TX -----

u08 *proto_cmd_api_tx_begin(u16 size)
{
  return mode_tx_begin(size);
}

u16 proto_cmd_api_tx_end(u16 size)
{
  return mode_tx_end(size);
}

// ----- rx packet from pio and send to parallel port -----

u16 proto_cmd_api_rx_size()
{
  return mode_rx_size();
}

u08 *proto_cmd_api_rx_begin(u16 size)
{
  return mode_rx_begin(size);
}

u16 proto_cmd_api_rx_end(u16 size)
{
  return mode_rx_end(size);
}



