#ifndef PROTO_CMD_H
#define PROTO_CMD_H

#include "proto_atom.h"

#define PROTO_RET_RX_TOO_LARGE    (PROTO_RET_CUSTOM + 0)
#define PROTO_RET_RX_ERROR        (PROTO_RET_CUSTOM + 1)
#define PROTO_RET_TX_ERROR        (PROTO_RET_CUSTOM + 2)

extern int proto_cmd_attach(proto_handle_t *proto);
extern int proto_cmd_detach(proto_handle_t *proto);

extern int proto_cmd_get_status(proto_handle_t *proto, UWORD *status);

extern int proto_cmd_send_frame(proto_handle_t *proto, UBYTE *buf, UWORD num_bytes);
extern int proto_cmd_recv_frame(proto_handle_t *proto, UBYTE *buf, UWORD max_bytes, UWORD *num_bytes);

#endif
