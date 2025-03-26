#ifndef PROTO_CMD_H
#define PROTO_CMD_H

#define PROTO_CMD_HANDLE_IDLE     0
#define PROTO_CMD_HANDLE_DONE     1
#define PROTO_CMD_HANDLE_UNKNOWN  2
#define PROTO_CMD_HANDLE_INIT     3
#define PROTO_CMD_HANDLE_EXIT     4
#define PROTO_CMD_HANDLE_RESET    5

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

#endif
