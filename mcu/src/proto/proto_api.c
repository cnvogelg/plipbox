#include "types.h"

#ifdef DEBUG_MODE
#define DEBUG
#endif

#include "debug.h"
#include "mode.h"
#include "hw_uart.h"
#include "uartutil.h"
#include "proto_cmd.h"
#include "proto_api.h"

#include "proto_event_shared.h"
#include "nic_shared.h"

static u16 event_mask;
static u08 trigger_update;
static u08 link_status;
static u08 nic_status;
static u16 req_event_mask;

static u16 rx_error;
static u16 rx_drop_count;
static u16 tx_error;
static u16 tx_drop_count;

void proto_api_init(void)
{
  proto_cmd_init();

  event_mask = 0;
  trigger_update = 0;
  link_status = NIC_LINK_STATUS_UNKNOWN;
  nic_status = NIC_STATUS_UNKNOWN;

  rx_error = 0;
  rx_drop_count = 0;
  tx_error = 0;
  tx_drop_count = 0;
  req_event_mask = 0;
}

u16 proto_api_cmd_get_version(void)
{
  return VERSION_MAJ << 8 | VERSION_MIN;
}

u16 proto_api_cmd_mode_attach(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("proto: attach"));
  uart_send_crlf();

  return mode_attach();
}

u16 proto_api_cmd_mode_detach(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("proto: detach"));
  uart_send_crlf();

  return mode_detach();
}

u16 proto_api_cmd_mode_get(void)
{
  return mode_get_attached_mode();
}

void proto_api_cmd_mode_set(u16 mode)
{
  mode_set_request_mode(mode);
}

// event mask stuff
u16 proto_api_cmd_event_mask(void)
{
  // clear mask on read
  u16 msk = event_mask;
  DS("event_mask:"); DW(msk); DNL;
  event_mask = 0;
  return msk;
}

void proto_api_add_event_mask(u16 mask)
{
  proto_api_set_event_mask(mask | event_mask);
}

void proto_api_set_event_mask(u16 mask)
{
  // nothing to do
  if(mask == event_mask) {
    return;
  }

  event_mask = mask;
  trigger_update = 1;
  DS("set_event_mask:"); DW(event_mask); DNL;
}

u16 proto_api_take_event_mask(void)
{
  // take and clear the mask
  // any pending trigger is removed
  u16 msk = event_mask;
  event_mask = 0;
  trigger_update = 0;
  return msk;
}

void proto_api_process_trigger(void)
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

void proto_api_set_rx_pending(void)
{
  proto_api_add_event_mask(PROTO_EVENT_RX_PENDING);
}

u16 proto_api_cmd_link_status(void)
{
  return link_status;
}

u08 proto_api_get_link_status(void)
{
  return link_status;
}

void proto_api_set_link_status(u08 status)
{
  link_status = status;
  DS("set_link_status:"); DW(status); DNL;
  proto_api_add_event_mask(PROTO_EVENT_LINK_STATUS);
}

u08 proto_api_get_nic_status(void)
{
  return nic_status;
}

u16 proto_api_cmd_nic_status(void)
{
  return nic_status;
}

void proto_api_set_nic_status(u08 status)
{
  nic_status = status;
  DS("set_nic_status:"); DW(status); DNL;
  proto_api_add_event_mask(PROTO_EVENT_NIC_STATUS);
}

u16 proto_api_cmd_req_event_mask(void)
{
  return req_event_mask;
}

void proto_api_set_req_event_mask(u16 req_mask)
{
  if(req_event_mask == req_mask) {
    return;
  }

  req_event_mask = req_mask;
  proto_api_add_event_mask(PROTO_EVENT_REQ_EVENTS);
}

void proto_api_add_req_event_mask(u16 req_mask)
{
  proto_api_set_req_event_mask(req_event_mask | req_mask);
}

// rx status

u16 proto_api_cmd_rx_error(void)
{
  return rx_error;
}

void proto_api_set_rx_error(u16 error)
{
  rx_error = error;
  DS("rx_error:"); DW(rx_error); DNL;
  if(rx_error != 0) {
    proto_api_add_event_mask(PROTO_EVENT_RX_ERROR);
  }
}

u16 proto_api_cmd_rx_drop_count(void)
{
  return rx_drop_count;
}

void proto_api_set_rx_drop_count(u16 drops)
{
  rx_drop_count = drops;
  DS("rx_drop_count:"); DW(rx_drop_count); DNL;
  proto_api_add_event_mask(PROTO_EVENT_RX_DROPS);
}

void proto_api_add_rx_drop_count(u16 drops)
{
  proto_api_set_rx_drop_count(rx_drop_count + drops);
}

// tx status

u16 proto_api_cmd_tx_error(void)
{
  return tx_error;
}

void proto_api_set_tx_error(u16 error)
{
  tx_error = error;
  DS("tx_error:"); DW(tx_error); DNL;
  if(tx_error != 0) {
    proto_api_add_event_mask(PROTO_EVENT_TX_ERROR);
  }
}

u16 proto_api_cmd_tx_drop_count(void)
{
  return tx_drop_count;
}

void proto_api_set_tx_drop_count(u16 drops)
{
  tx_drop_count = drops;
  DS("tx_drop_count:"); DW(tx_drop_count); DNL;
  proto_api_add_event_mask(PROTO_EVENT_TX_DROPS);
}

void proto_api_add_tx_drop_count(u16 drops)
{
  proto_api_set_tx_drop_count(tx_drop_count + drops);
}

// ----- ping -----

void proto_api_cmd_ping(void)
{
  mode_ping();
}

// ----- TX -----

u08 *proto_api_cmd_tx_begin(u16 size)
{
  return mode_tx_begin(size);
}

u16 proto_api_cmd_tx_end(u16 size)
{
  return mode_tx_end(size);
}

// ----- rx packet from pio and send to parallel port -----

u16 proto_api_cmd_rx_size()
{
  return mode_rx_size();
}

u08 *proto_api_cmd_rx_begin(u16 size)
{
  return mode_rx_begin(size);
}

u16 proto_api_cmd_rx_end(u16 size)
{
  return mode_rx_end(size);
}
