#ifndef PROTO_CMD_H
#define PROTO_CMD_H

#define PROTO_CMD_HANDLE_IDLE     0
#define PROTO_CMD_HANDLE_DONE     1
#define PROTO_CMD_HANDLE_UNKNOWN  2
#define PROTO_CMD_HANDLE_INIT     3
#define PROTO_CMD_HANDLE_EXIT     4

// protocol state
#define PROTO_CMD_STATE_IDLE    0
#define PROTO_CMD_STATE_RX      1
#define PROTO_CMD_STATE_TX      2
#define PROTO_CMD_STATE_REQ     3

extern void proto_cmd_init(void);
extern u08  proto_cmd_handle_init(void);
extern u08  proto_cmd_handle_main(void);

extern void proto_cmd_trigger_status(void);
extern u08  proto_cmd_get_state(void);

// ----- API calls (defined in your app) -----
extern void proto_cmd_api_attach(void);
extern void proto_cmd_api_detach(void);

extern u16  proto_cmd_api_get_status(void);
extern void proto_cmd_api_ping(void);
extern u16  proto_cmd_api_get_version(void);

extern u08  proto_cmd_api_rx_size(u16 *go_size);
extern u08 *proto_cmd_api_rx_begin(u16 size);
extern u08  proto_cmd_api_rx_end(u16 size);

extern u08 *proto_cmd_api_tx_begin(u16 size);
extern u08  proto_cmd_api_tx_end(u16 size);

// ----- request -----
struct proto_cmd_req {
  u08 command;
  u08 in_extra;
  u08 status;
  u08 out_extra;
  u16 in_size;
  u16 out_size;
  u08 *in_buf;
  u08 *out_buf;
};
typedef struct proto_cmd_req proto_cmd_req_t;

extern void proto_cmd_api_req_in(proto_cmd_req_t *req);
extern void proto_cmd_api_req_out(proto_cmd_req_t *req);

#endif
