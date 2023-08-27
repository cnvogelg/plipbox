#ifndef PROTO_CMD_H
#define PROTO_CMD_H

#define PROTO_CMD_HANDLE_IDLE   0
#define PROTO_CMD_HANDLE_DONE   1
#define PROTO_CMD_HANDLE_RESET  2

// protocol state
#define PROTO_CMD_STATE_IDLE    0
#define PROTO_CMD_STATE_RX      1
#define PROTO_CMD_STATE_TX      2

extern void proto_cmd_init(void);
extern u08  proto_cmd_handle(void);

extern void proto_cmd_trigger_status(void);
extern u08  proto_cmd_get_state(void);

// ----- API calls (defined in your app) -----
extern void proto_cmd_api_attach(void);
extern void proto_cmd_api_detach(void);

extern u16  proto_cmd_api_get_status(void);

extern u08  proto_cmd_api_rx_size(u16 *size);
extern u08 *proto_cmd_api_rx_begin(u16 size);
extern u08  proto_cmd_api_rx_end(u16 size);

extern u08 *proto_cmd_api_tx_begin(u16 size);
extern u08  proto_cmd_api_tx_end(u16 size);

extern u16  proto_cmd_api_get_version(void);

extern void proto_cmd_api_set_mode(u16 mode);
extern u16  proto_cmd_api_get_mode(void);

extern void proto_cmd_api_set_mac(mac_t mac);
extern void proto_cmd_api_get_mac(mac_t mac);
extern void proto_cmd_api_get_def_mac(mac_t mac);

#endif
