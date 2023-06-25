#ifndef PROTO_CMD_H
#define PROTO_CMD_H

#define PROTO_CMD_HANDLE_IDLE   0
#define PROTO_CMD_HANDLE_DONE   1
#define PROTO_CMD_HANDLE_RESET  2

extern void proto_cmd_init(void);
extern u08  proto_cmd_handle(void);

extern void proto_cmd_set_status(u16 status, u08 notify);
extern u16  proto_cmd_get_status(void);
extern void proto_cmd_set_status_mask(u16 mask, u08 notify);
extern void proto_cmd_clr_status_mask(u16 mask, u08 notify);

// ----- API calls (defined in your app) -----
extern void proto_cmd_api_attach(void);
extern void proto_cmd_api_detach(void);

extern u08  proto_cmd_api_rx_size(u16 *size);
extern u08 *proto_cmd_api_rx_begin(u16 size);
extern u08  proto_cmd_api_rx_end(u16 size);

extern u08 *proto_cmd_api_tx_begin(u16 size);
extern u08  proto_cmd_api_tx_end(u16 size);

#endif
