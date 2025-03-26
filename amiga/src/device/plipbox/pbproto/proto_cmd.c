#define __NOLIBBASE__
#include <proto/exec.h>

#include "compiler.h"
#include "debug.h"

#include "proto_cmd.h"
#include "proto_cmd_shared.h"
#include "param_shared.h"

// low level ops

int proto_cmd_reset(proto_handle_t *proto)
{
  int res;

  d8(("proto_cmd_reset:"));
  res = proto_atom_action(proto, PROTO_CMD_RESET);
  d8r((" res=%ld\n", (LONG)res));
  return res;
}

int proto_cmd_alive(proto_handle_t *proto)
{
  int res;

  d8(("proto_cmd_alive:"));
  res = proto_atom_action_no_busy(proto, PROTO_CMD_ALIVE);
  d8r((" res=%ld\n", (LONG)res));
  return res;
}

// driver control

int proto_cmd_init(proto_handle_t *proto, UWORD token)
{
  int res;

  d8(("proto_cmd_init: token=%lx", (ULONG)token));
  res = proto_atom_write_word(proto, PROTO_CMD_INIT, token);
  d8r((" res=%ld\n", (LONG)res));
  return res;
}

int proto_cmd_ping(proto_handle_t *proto, UWORD *token)
{
  int res;

  d8(("proto_cmd_ping:"));
  res = proto_atom_read_word(proto, PROTO_CMD_PING, token);
  d8r((" token=%lx res=%ld\n", (ULONG)*token, (LONG)res));
  return res;
}

int proto_cmd_exit(proto_handle_t *proto)
{
  int res;

  d8(("proto_cmd_exit:"));
  res = proto_atom_action(proto, PROTO_CMD_EXIT);
  d8r((" res=%ld\n", (LONG)res));
  return res;
}

int proto_cmd_get_version(proto_handle_t *proto, UWORD *version)
{
  int res;

  d8(("proto_cmd_get_version:"));
  res = proto_atom_read_word(proto, PROTO_CMD_GET_VERSION, version);
  d8r((" version=%lx res=%ld\n", (ULONG)*version, (LONG)res));
  return res;
}

// mode

int proto_cmd_mode_attach(proto_handle_t *proto, UWORD *result)
{
  int res;

  d8(("proto_cmd_mode_attach:"));
  res = proto_atom_read_word(proto, PROTO_CMD_MODE_ATTACH, result);
  d8r((" mode=%lx res=%ld\n", (ULONG)*result, (LONG)res));
  return res;
}

int proto_cmd_mode_detach(proto_handle_t *proto, UWORD *result)
{
  int res;

  d8(("proto_cmd_mode_detach:"));
  res = proto_atom_read_word(proto, PROTO_CMD_MODE_DETACH, result);
  d8r((" mode=%lx res=%ld\n", (ULONG)*result, (LONG)res));
  return res;
}

int proto_cmd_mode_set(proto_handle_t *proto, UWORD mode)
{
  int res;

  d8(("proto_cmd_mode_set: mode=%lx", (ULONG)mode));
  res = proto_atom_write_word(proto, PROTO_CMD_MODE_SET, mode);
  d8r((" res=%ld\n", (LONG)res));
  return res;
}

int proto_cmd_mode_get(proto_handle_t *proto, UWORD *mode)
{
  int res;

  d8(("proto_cmd_mode_get:"));
  res = proto_atom_read_word(proto, PROTO_CMD_MODE_GET, mode);
  d8r((" mode=%lx res=%ld\n", (ULONG)*mode, (LONG)res));
  return res;
}

// status

int proto_cmd_event_mask(proto_handle_t *proto, UWORD *event_mask)
{
  int res;

  d8(("proto_cmd_event_mask:"));
  res = proto_atom_read_word(proto, PROTO_CMD_EVENT_MASK, event_mask);
  d8r((" mask=%lx res=%ld\n", (ULONG)*event_mask, (LONG)res));
  return res;
}

int proto_cmd_link_status(proto_handle_t *proto, UWORD *link_status)
{
  int res;

  d8(("proto_cmd_nic_link_status:"));
  res = proto_atom_read_word(proto, PROTO_CMD_LINK_STATUS, link_status);
  d8r((" status=%lx res=%ld\n", (ULONG)*link_status, (LONG)res));
  return res;
}

int proto_cmd_nic_status(proto_handle_t *proto, UWORD *nic_status)
{
  int res;

  d8(("proto_cmd_nic_status:"));
  res = proto_atom_read_word(proto, PROTO_CMD_NIC_STATUS, nic_status);
  d8r((" status=%lx res=%ld\n", (ULONG)*nic_status, (LONG)res));
  return res;
}

int proto_cmd_mode_status(proto_handle_t *proto, UWORD *mode_status)
{
  int res;

  d8(("proto_cmd_mode_status:"));
  res = proto_atom_read_word(proto, PROTO_CMD_MODE_STATUS, mode_status);
  d8r((" status=%lx res=%ld\n", (ULONG)*mode_status, (LONG)res));
  return res;
}

// ----- Request -----

int proto_cmd_request(proto_handle_t *proto, proto_api_req_t *req)
{
  int res;
  ULONG data;

  // begin command
  d8(("proto_cmd_request:"));
  UWORD in_size = req->in_size;
  data = req->command | (req->in_extra << 8) | (in_size << 16);
  d8r((" data_in=%lx\n", data));
  res = proto_atom_write_long(proto, PROTO_CMD_REQ_IN, data);
  d8r((" res_in=%ld", (LONG)res));
  if(res != PROTO_RET_OK) {
    return res;
  }

  // has input data
  if(in_size > 0) {
    // round to even
    if((in_size & 1) != 0) {
      in_size++;
    }
    res = proto_atom_write_block(proto, PROTO_CMD_REQ_IN_DATA, req->in_buf, in_size);
    d8r((" res_in_data=%ld", (LONG)res));
    if(res != PROTO_RET_OK) {
      return res;
    }
  }

  // retrieve result
  res = proto_atom_read_long(proto, PROTO_CMD_REQ_OUT, &data);
  d8r((" data_out=%lx res_out=%ld", data, (LONG)res));
  if(res != PROTO_RET_OK) {
    return res;
  }
  UWORD out_size = (UWORD)(data >> 16);
  UBYTE status = (UBYTE)(data & 0xff);
  UBYTE out_extra = (UBYTE)((data >> 8) & 0xff);
  req->status = status;
  req->out_size = out_size;
  req->out_extra = out_extra;
  d8r((" out_size=%ld out_extra=%ld status=%ld", (ULONG)out_size, (ULONG)out_extra, (ULONG)status));

  // has output data?
  if(out_size > 0) {
    // round to even
    if((out_size & 1) != 0) {
      out_size++;
    }
    res = proto_atom_read_block(proto, PROTO_CMD_REQ_OUT_DATA, req->out_buf, out_size);
    d8r((" res_out_data=%ld\n", (LONG)res));
    if(res != PROTO_RET_OK) {
      return res;
    }
  } else {
    d8r(("\n"));
  }

  return PROTO_RET_OK;
}

int proto_cmd_request_event_mask(proto_handle_t *proto, UWORD *req_event_mask)
{
  int res;

  d8(("proto_cmd_request_event_mask:"));
  res = proto_atom_read_word(proto, PROTO_CMD_REQ_EVENT_MASK, req_event_mask);
  d8r((" events=%lx res=%ld\n", *req_event_mask, (LONG)res));

  return res;
}

// ----- RX/TX -----

int proto_cmd_send_frame(proto_handle_t *proto, UBYTE *buf, UWORD num_bytes, UWORD *ret_event_mask)
{
  int res;

  d8(("proto_cmd_send_frame: size=%lu", (ULONG)num_bytes));

  res = proto_atom_write_word(proto, PROTO_CMD_TX_SIZE, num_bytes);
  d8r((", size: res=%ld", (LONG)res));
  if (res != PROTO_RET_OK)
  {
    return res;
  }

  /* align size to be even */
  if (num_bytes & 1)
  {
    num_bytes++;
  }

  res = proto_atom_write_block(proto, PROTO_CMD_TX_BUF, buf, num_bytes);
  d8r((", block: size=%lu res=%ld", (ULONG)num_bytes, (LONG)res));
  if (res != PROTO_RET_OK)
  {
    return res;
  }

  res = proto_atom_read_word(proto, PROTO_CMD_TX_RESULT, ret_event_mask);
  d8r((", event_mask=%lx res=%ld\n", (ULONG)*ret_event_mask, (LONG)res));
  if (res != PROTO_RET_OK)
  {
    return res;
  }

  return PROTO_RET_OK;
}

int proto_cmd_recv_frame(proto_handle_t *proto, UBYTE *buf, UWORD max_bytes, UWORD *num_bytes, UWORD *ret_event_mask)
{
  int res;

  d8(("proto_cmd_recv_frame: max=%lu", (ULONG)max_bytes));

  res = proto_atom_read_word(proto, PROTO_CMD_RX_SIZE, num_bytes);
  d8r((", size=%ld res=%ld", (LONG)*num_bytes, (LONG)res));
  if (res != PROTO_RET_OK)
  {
    return res;
  }

  /* check max size */
  UWORD rx_bytes = *num_bytes;
  if (rx_bytes > max_bytes)
  {
    return PROTO_RET_RX_TOO_LARGE;
  }

  /* anything to read? */
  if (rx_bytes > 0)
  {

    /* align size to be even */
    if (rx_bytes & 1)
    {
      rx_bytes++;
    }

    res = proto_atom_read_block(proto, PROTO_CMD_RX_BUF, buf, rx_bytes);
    d8r((", block: size=%lu res=%ld", (ULONG)rx_bytes, (LONG)res));
    if (res != PROTO_RET_OK)
    {
      return res;
    }
  }

  res = proto_atom_read_word(proto, PROTO_CMD_RX_RESULT, ret_event_mask);
  d8r((", event_mask=%lx res=%ld\n", (ULONG)*ret_event_mask, (LONG)res));
  if (res != PROTO_RET_OK)
  {
    return res;
  }

  return PROTO_RET_OK;
}

int proto_cmd_rx_error(proto_handle_t *proto, UWORD *rx_error)
{
  int res;

  d8(("proto_cmd_rx_error:"));
  res = proto_atom_read_word(proto, PROTO_CMD_RX_ERROR, rx_error);
  d8r((" rx_error=%lx res=%ld\n", (ULONG)*rx_error, (LONG)res));
  return res;
}

int proto_cmd_tx_error(proto_handle_t *proto, UWORD *tx_error)
{
  int res;

  d8(("proto_cmd_tx_error:"));
  res = proto_atom_read_word(proto, PROTO_CMD_TX_ERROR, tx_error);
  d8r((" tx_error=%lx res=%ld\n", (ULONG)*tx_error, (LONG)res));
  return res;
}

int proto_cmd_rx_drop_count(proto_handle_t *proto, UWORD *rx_drops)
{
  int res;

  d8(("proto_cmd_rx_drops:"));
  res = proto_atom_read_word(proto, PROTO_CMD_RX_DROP_COUNT, rx_drops);
  d8r((" drops=%lx res=%ld\n", (ULONG)*rx_drops, (LONG)res));
  return res;
}

int proto_cmd_tx_drop_count(proto_handle_t *proto, UWORD *tx_drops)
{
  int res;

  d8(("proto_cmd_tx_drops:"));
  res = proto_atom_read_word(proto, PROTO_CMD_TX_DROP_COUNT, tx_drops);
  d8r((" drops=%lx res=%ld\n", (ULONG)*tx_drops, (LONG)res));
  return res;
}
