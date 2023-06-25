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
static u16 cmd_status;

void proto_cmd_init(void)
{
  proto_atom_init();
  cmd_status = 0;
  tx_size = 0;
  rx_size = 0;
  tx_status = 0;
  rx_status = 0;
}

void proto_cmd_set_status(u16 status, u08 notify)
{
  DS("STAT:"); DW(status);
  cmd_status = status;
  if(notify) {
    DS(" !");
    proto_atom_pulse_irq();
  }
  DNL;
}

u16 proto_cmd_get_status(void)
{
  return cmd_status;
}

void proto_cmd_set_status_mask(u16 mask, u08 notify)
{
  proto_cmd_set_status(cmd_status | mask, notify);
}

void proto_cmd_clr_status_mask(u16 mask, u08 notify)
{
  proto_cmd_set_status(cmd_status & ~mask, notify);
}

u08 proto_cmd_handle(void)
{
  u08 cmd = proto_atom_get_cmd();
  if(cmd == PROTO_NO_CMD) {
    return PROTO_CMD_HANDLE_IDLE;
  }

  DS("CMD:");
  DB(cmd); DC(' ');
  u08 result = PROTO_CMD_HANDLE_DONE;

  switch(cmd) {
    case PROTO_CMD_HANDLE_RESET:
      result = PROTO_CMD_HANDLE_RESET;
      break;

    case PROTO_CMD_ATTACH:
      DS("attach"); DNL;
      proto_atom_action();
      proto_cmd_api_attach();
      break;
    case PROTO_CMD_DETACH:
      DS("detach"); DNL;
      proto_atom_action();
      proto_cmd_api_detach();
      break;

    case PROTO_CMD_GET_STATUS: {
      DS("get_status:"); DW(cmd_status); DNL;
      proto_atom_read_word(cmd_status);
      break;
    }

    // ----- tx -----
    case PROTO_CMD_TX_SIZE: {
      DS("tx_size:"); DW(tx_size); DNL;
      tx_size = proto_atom_write_word();
      break;
    }
    case PROTO_CMD_TX_BUF: {
      u08 *buf = proto_cmd_api_tx_begin(tx_size);
      DS("tx_buf:"); DW(tx_size); DC(','); DP(buf); DNL;
      proto_atom_write_block(buf, tx_size);
      tx_status = proto_cmd_api_tx_end(tx_size);
      DS("tx_bufe:"); DB(tx_status); DNL;
      break;
    }
    case PROTO_CMD_TX_RESULT:
      DS("tx_res:"); DB(tx_status); DNL;
      proto_atom_read_word(tx_status);
      break;

    // ----- rx -----
    case PROTO_CMD_RX_SIZE: {
      rx_status = proto_cmd_api_rx_size(&rx_size);
      DS("rx_size:"); DB(rx_status); DC(','); DW(rx_size); DNL;
      proto_atom_read_word(rx_size);
      break;
    }
    case PROTO_CMD_RX_BUF: {
      u08 *buf = proto_cmd_api_rx_begin(rx_size);
      DS("rx_buf:"); DW(rx_size); DC(','); DP(buf); DNL;
      proto_atom_read_block(buf, rx_size);
      rx_status = proto_cmd_api_rx_end(rx_size);
      DS("rx_bufe:"); DB(rx_status); DNL;
      break;
    }
    case PROTO_CMD_RX_RESULT:
      DS("rx_res:"); DB(rx_status); DNL;
      proto_atom_read_word(rx_status);
      break;

    default:
      DC('?'); DNL;
      break;
  }

  return result;
}
