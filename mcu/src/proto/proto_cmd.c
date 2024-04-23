#include "types.h"

#ifdef DEBUG_PROTO_CMD
#define DEBUG
#endif

#include "proto_atom.h"
#include "proto_cmd.h"
#include "proto_cmd_shared.h"
#include "debug.h"

static u16 size;
static u16 status;
static u08 cmd_state = PROTO_CMD_STATE_IDLE;
static u16 token = 0;

static proto_cmd_req_t req;

void proto_cmd_init(void)
{
  proto_atom_init();
  size = 0;
  status = 0;
  cmd_state = PROTO_CMD_STATE_IDLE;
  token = 0;
}

void proto_cmd_trigger_status(void)
{
  DT; DS("TRIG!!!"); DNL;
  proto_atom_pulse_irq();
}

u08 proto_cmd_get_state(void)
{
  return cmd_state;
}

#ifdef DEBUG
#define CHECK_STATE(state) \
{ if(cmd_state != state) { DS("STATE???"); DB(state); } }
#else
#define CHECK_STATE(state)
#endif

u08 proto_cmd_handle_init(void)
{
  u08 cmd = proto_atom_get_cmd();
  if(cmd == PROTO_NO_CMD) {
    return PROTO_CMD_HANDLE_IDLE;
  }

  DT; DS("CMD:");
  DB(cmd); DC(' ');
  u08 result = PROTO_CMD_HANDLE_DONE;

  // we are only waiting for an init command
  switch(cmd) {
    case PROTO_CMD_INIT:
      DS("INIT:");
      token = proto_atom_write_word();
      DW(token); DNL;
      result = PROTO_CMD_HANDLE_INIT;
      break;
    case PROTO_CMD_PING:
      DS("PING:"); DW(token); DNL;
      proto_atom_read_word(token);
      proto_cmd_api_ping();
      break;
    case PROTO_CMD_RESET:
      DS("RESET"); DNL;
      proto_atom_action();
      return PROTO_CMD_HANDLE_RESET;
    case PROTO_CMD_ALIVE:
      DS("ALIVE"); DNL;
      proto_atom_action();
      break;
    default:
      DC('?'); DNL;
      result = PROTO_CMD_HANDLE_UNKNOWN;
      break;
  }
  return result;
}


u08 proto_cmd_handle_main(void)
{
  u08 cmd = proto_atom_get_cmd();
  if(cmd == PROTO_NO_CMD) {
    return PROTO_CMD_HANDLE_IDLE;
  }

  DT; DS("CMD:");
  DB(cmd); DC(' ');
  u08 result = PROTO_CMD_HANDLE_DONE;

  switch(cmd) {
    case PROTO_CMD_RESET:
      DS("RESET"); DNL;
      proto_atom_action();
      return PROTO_CMD_HANDLE_RESET;
    case PROTO_CMD_INIT:
      DS("INIT:");
      token = proto_atom_write_word();
      DW(token); DNL;
      result = PROTO_CMD_HANDLE_INIT;
      break;
    case PROTO_CMD_PING:
      DS("PING:"); DW(token); DNL;
      proto_atom_read_word(token);
      proto_cmd_api_ping();
      break;
    case PROTO_CMD_EXIT:
      DS("EXIT"); DNL;
      proto_atom_action();
      result = PROTO_CMD_HANDLE_EXIT;
      break;

    case PROTO_CMD_ATTACH:
      CHECK_STATE(PROTO_CMD_STATE_IDLE);
      DS("attach"); DNL;
      proto_atom_action();
      proto_cmd_api_attach();
      break;
    case PROTO_CMD_DETACH:
      CHECK_STATE(PROTO_CMD_STATE_IDLE);
      DS("detach"); DNL;
      proto_atom_action();
      proto_cmd_api_detach();
      break;

    // ----- events -----
    case PROTO_CMD_EVENT_MASK: {
      CHECK_STATE(PROTO_CMD_STATE_IDLE);
      u16 event_mask = proto_cmd_api_event_mask();
      DS("event_mask:"); DW(event_mask); DNL;
      proto_atom_read_word(event_mask);
      break;
    }
    case PROTO_CMD_LINK_STATUS: {
      CHECK_STATE(PROTO_CMD_STATE_IDLE);
      u16 link_status = proto_cmd_api_link_status();
      DS("link_status:"); DW(link_status); DNL;
      proto_atom_read_word(link_status);
      break;
    }
    case PROTO_CMD_HW_STATUS: {
      CHECK_STATE(PROTO_CMD_STATE_IDLE);
      u16 hw_status = proto_cmd_api_hw_status();
      DS("hw_status:"); DW(hw_status); DNL;
      proto_atom_read_word(hw_status);
      break;
    }

    // ----- tx -----
    case PROTO_CMD_TX_SIZE: {
      CHECK_STATE(PROTO_CMD_STATE_IDLE);
      cmd_state = PROTO_CMD_STATE_TX;
      size = proto_atom_write_word();
      DS("tx_size:"); DW(size); DNL;
      break;
    }
    case PROTO_CMD_TX_BUF: {
      CHECK_STATE(PROTO_CMD_STATE_TX);
      u08 *buf = proto_cmd_api_tx_begin(size);
      u16 even_size = size;
      if(even_size & 1) {
        even_size++;
      }
      DS("tx_buf:"); DW(size); DC('_'); DW(even_size); DC(','); DP(buf); DNL;
      proto_atom_write_block(buf, even_size);
      status = proto_cmd_api_tx_end(size);
      DT; DS("tx_bufe:"); DW(status); DNL;
      break;
    }
    case PROTO_CMD_TX_RESULT: {
      CHECK_STATE(PROTO_CMD_STATE_TX);
      cmd_state = PROTO_CMD_STATE_IDLE;
      DS("tx_res:"); DW(status); DNL;
      proto_atom_read_word(status);
      break;
    }
    case PROTO_CMD_TX_ERROR: {
      CHECK_STATE(PROTO_CMD_STATE_IDLE);
      u16 tx_error = proto_cmd_api_tx_error();
      DS("tx_error:"); DW(tx_error); DNL;
      proto_atom_read_word(tx_error);
      break;
    }
    case PROTO_CMD_TX_DROP_COUNT: {
      CHECK_STATE(PROTO_CMD_STATE_IDLE);
      u16 tx_drop_count = proto_cmd_api_tx_drop_count();
      DS("tx_drop_count:"); DW(tx_drop_count); DNL;
      proto_atom_read_word(tx_drop_count);
      break;
    }

    // ----- rx -----
    case PROTO_CMD_RX_SIZE: {
      CHECK_STATE(PROTO_CMD_STATE_IDLE);
      cmd_state = PROTO_CMD_STATE_RX;
      size = proto_cmd_api_rx_size();
      DS("rx_size:"); DW(size); DNL;
      proto_atom_read_word(size);
      break;
    }
    case PROTO_CMD_RX_BUF: {
      CHECK_STATE(PROTO_CMD_STATE_RX);
      u08 *buf = proto_cmd_api_rx_begin(size);
      u16 even_size = size;
      if(even_size & 1) {
        even_size++;
      }
      DS("rx_buf:"); DW(size); DC('_'); DW(even_size); DC(','); DP(buf); DNL;
      proto_atom_read_block(buf, even_size);
      status = proto_cmd_api_rx_end(size);
      DT; DS("rx_bufe:"); DW(status); DNL;
      break;
    }
    case PROTO_CMD_RX_RESULT: {
      CHECK_STATE(PROTO_CMD_STATE_RX);
      cmd_state = PROTO_CMD_STATE_IDLE;
      DS("rx_res:"); DW(status); DNL;
      proto_atom_read_word(status);
      break;
    }
    case PROTO_CMD_RX_ERROR: {
      CHECK_STATE(PROTO_CMD_STATE_IDLE);
      u16 rx_error = proto_cmd_api_rx_error();
      DS("rx_error:"); DW(rx_error); DNL;
      proto_atom_read_word(rx_error);
      break;
    }
    case PROTO_CMD_RX_DROP_COUNT: {
      CHECK_STATE(PROTO_CMD_STATE_IDLE);
      u16 rx_drop_count = proto_cmd_api_rx_drop_count();
      DS("rx_drop_count:"); DW(rx_drop_count); DNL;
      proto_atom_read_word(rx_drop_count);
      break;
    }

    // ----- param -----
    case PROTO_CMD_GET_VERSION: {
      u16 version = proto_cmd_api_get_version();
      proto_atom_read_word(version);
      DS("get_version:"); DW(version); DNL;
      break;
    }

    // ----- req -----
    case PROTO_CMD_REQ_IN: {
      CHECK_STATE(PROTO_CMD_STATE_IDLE);
      cmd_state = PROTO_CMD_STATE_REQ;
      u32 data = proto_atom_write_long();
      req.command = (u08)(data & 0xff);
      req.in_extra = (u08)((data >> 8) & 0xff);
      req.in_size = (u16)(data >> 16);
      DS("req_in:cmd="); DB(req.command); DC('/'); DB(req.in_extra); DC('+'); DW(req.in_size); DNL;
      proto_cmd_api_req_in(&req);
      break;
    }
    case PROTO_CMD_REQ_IN_DATA: {
      CHECK_STATE(PROTO_CMD_STATE_REQ);
      DS("req_in_data:"); DW(req.in_size); DNL;
      proto_atom_write_block(req.in_buf, req.in_size);
      break;
    }
    case PROTO_CMD_REQ_OUT: {
      CHECK_STATE(PROTO_CMD_STATE_REQ);
      proto_cmd_api_req_out(&req);
      u32 data = (u32)req.status | ((u32)req.out_extra << 8) | ((u32)req.out_size << 16);
      proto_atom_read_long(data);
      DS("req_out:res="); DB(req.status); DC('/'); DB(req.out_extra); DC('+'); DW(req.out_size); DNL;
      if(req.out_size == 0) {
        cmd_state = PROTO_CMD_STATE_IDLE;
      }
      break;
    }
    case PROTO_CMD_REQ_OUT_DATA: {
      CHECK_STATE(PROTO_CMD_STATE_REQ);
      DS("req_out_data:"); DW(req.out_size); DNL;
      proto_atom_read_block(req.out_buf, req.out_size);
      cmd_state = PROTO_CMD_STATE_IDLE;
      break;
    }
    case PROTO_CMD_REQ_EVENT_MASK: {
      CHECK_STATE(PROTO_CMD_STATE_IDLE);
      u16 event_mask = proto_cmd_api_req_event_mask();
      DS("req_event_mask:"); DW(event_mask); DNL;
      proto_atom_read_word(event_mask);
      break;
    }

    default:
      DC('?'); DNL;
      result = PROTO_CMD_HANDLE_UNKNOWN;
      break;
  }

  return result;
}
