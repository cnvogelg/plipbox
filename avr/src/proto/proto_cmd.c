#include "global.h"

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

void proto_cmd_init(void)
{
  proto_atom_init();
  tx_size = 0;
  rx_size = 0;
  tx_status = 0;
  rx_status = 0;
}

void proto_cmd_trigger_status(void)
{
  DS("trigger!"); DNL;
  proto_atom_pulse_irq();
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
      u16 status = proto_cmd_api_get_status();
      DS("get_status:"); DW(status); DNL;
      proto_atom_read_word(status);
      break;
    }

    // ----- tx -----
    case PROTO_CMD_TX_SIZE: {
      tx_size = proto_atom_write_word();
      DS("tx_size:"); DW(tx_size); DNL;
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

    // ----- config -----
    case PROTO_CMD_SET_MODE: {
      u16 mode = proto_atom_write_word();
      DS("set_mode:"); DW(mode); DNL;
      proto_cmd_api_set_mode(mode);
      break;
    }
    case PROTO_CMD_GET_MODE: {
      u16 mode = proto_cmd_api_get_mode();
      proto_atom_read_word(mode);
      DS("get_mode:"); DW(mode); DNL;
      break;
    }
    case PROTO_CMD_SET_MAC: {
      mac_t mac;
      proto_atom_write_block(mac, MAC_SIZE);
      DS("set_mac:"); DM(mac); DNL;
      proto_cmd_api_set_mac(mac);
      break;
    }
    case PROTO_CMD_GET_MAC: {
      mac_t mac;
      proto_cmd_api_get_mac(mac);
      DS("get_mac:"); DM(mac); DNL;
      proto_atom_read_block(mac, MAC_SIZE);
      break;
    }
    case PROTO_CMD_GET_DEF_MAC: {
      mac_t mac;
      proto_cmd_api_get_def_mac(mac);
      DS("get_def_mac:"); DM(mac); DNL;
      proto_atom_read_block(mac, MAC_SIZE);
      break;
    }

    default:
      DC('?'); DNL;
      break;
  }

  return result;
}
