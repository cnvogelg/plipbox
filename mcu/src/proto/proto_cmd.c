#include "types.h"

#ifdef DEBUG_PROTO_CMD
#define DEBUG
#endif

#include "proto_atom.h"
#include "proto_cmd.h"
#include "proto_cmd_shared.h"
#include "debug.h"

static u16 tx_size;
static u16 rx_size;
static u08 tx_status;
static u08 rx_status;
static u08 cmd_state = PROTO_CMD_STATE_IDLE;
static u16 token = 0;

static proto_cmd_req_t req;

void proto_cmd_init(void)
{
  proto_atom_init();
  tx_size = 0;
  rx_size = 0;
  tx_status = 0;
  rx_status = 0;
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

    case PROTO_CMD_GET_STATUS: {
      CHECK_STATE(PROTO_CMD_STATE_IDLE);
      u16 status = proto_cmd_api_get_status();
      DS("get_status:"); DW(status); DNL;
      proto_atom_read_word(status);
      break;
    }

    // ----- tx -----
    case PROTO_CMD_TX_SIZE: {
      CHECK_STATE(PROTO_CMD_STATE_IDLE);
      cmd_state = PROTO_CMD_STATE_TX;
      tx_size = proto_atom_write_word();
      DS("tx_size:"); DW(tx_size); DNL;
      break;
    }
    case PROTO_CMD_TX_BUF: {
      CHECK_STATE(PROTO_CMD_STATE_TX);
      u08 *buf = proto_cmd_api_tx_begin(tx_size);
      u16 size = tx_size;
      if(size & 1) {
        size++;
      }
      DS("tx_buf:"); DW(tx_size); DC('_'); DW(size); DC(','); DP(buf); DNL;
      proto_atom_write_block(buf, size);
      tx_status = proto_cmd_api_tx_end(tx_size);
      DT; DS("tx_bufe:"); DB(tx_status); DNL;
      break;
    }
    case PROTO_CMD_TX_RESULT:
      CHECK_STATE(PROTO_CMD_STATE_TX);
      cmd_state = PROTO_CMD_STATE_IDLE;
      DS("tx_res:"); DB(tx_status); DNL;
      proto_atom_read_word(tx_status);
      break;

    // ----- rx -----
    case PROTO_CMD_RX_SIZE: {
      CHECK_STATE(PROTO_CMD_STATE_IDLE);
      cmd_state = PROTO_CMD_STATE_RX;
      rx_size = 0;
      rx_status = proto_cmd_api_rx_size(&rx_size);
      DS("rx_size:"); DB(rx_status); DC(':'); DW(rx_size); DNL;
      proto_atom_read_word(rx_size);
      break;
    }
    case PROTO_CMD_RX_BUF: {
      CHECK_STATE(PROTO_CMD_STATE_RX);
      u08 *buf = proto_cmd_api_rx_begin(rx_size);
      u16 size = rx_size;
      if(size & 1) {
        size++;
      }
      DS("rx_buf:"); DW(rx_size); DC('_'); DW(size); DC(','); DP(buf); DNL;
      proto_atom_read_block(buf, size);
      rx_status = proto_cmd_api_rx_end(rx_size);
      DT; DS("rx_bufe:"); DB(rx_status); DNL;
      break;
    }
    case PROTO_CMD_RX_RESULT:
      CHECK_STATE(PROTO_CMD_STATE_RX);
      cmd_state = PROTO_CMD_STATE_IDLE;
      DS("rx_res:"); DB(rx_status); DNL;
      proto_atom_read_word(rx_status);
      break;

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

    default:
      DC('?'); DNL;
      result = PROTO_CMD_HANDLE_UNKNOWN;
      break;
  }

  return result;
}
