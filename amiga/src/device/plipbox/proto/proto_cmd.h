#ifndef PROTO_CMD_H
#define PROTO_CMD_H

#include "proto_atom.h"

typedef UBYTE mac_t[6];
#define MAC_SIZE 6

#define PROTO_RET_RX_TOO_LARGE (PROTO_RET_CUSTOM + 0)
#define PROTO_RET_RX_ERROR (PROTO_RET_CUSTOM + 1)
#define PROTO_RET_TX_ERROR (PROTO_RET_CUSTOM + 2)

struct proto_cmd_req
{
  UBYTE command;   // in
  UBYTE in_extra;  // in
  UBYTE status;    // out
  UBYTE out_extra; // out
  UWORD in_size;   // in
  UWORD out_size;  // in: max_size, out: real_size
  APTR  in_buf;
  APTR  out_buf;
};
typedef struct proto_cmd_req proto_cmd_req_t;

extern int proto_cmd_init(proto_handle_t *proto, UWORD token);
extern int proto_cmd_ping(proto_handle_t *proto, UWORD *token);
extern int proto_cmd_exit(proto_handle_t *proto);
extern int proto_cmd_get_version(proto_handle_t *proto, UWORD *version);

extern int proto_cmd_attach(proto_handle_t *proto);
extern int proto_cmd_detach(proto_handle_t *proto);
extern int proto_cmd_get_status(proto_handle_t *proto, UWORD *status);

extern int proto_cmd_send_frame(proto_handle_t *proto, UBYTE *buf, UWORD num_bytes, UWORD *status);
extern int proto_cmd_recv_frame(proto_handle_t *proto, UBYTE *buf, UWORD max_bytes, UWORD *num_bytes, UWORD *status);

extern int proto_cmd_request(proto_handle_t *proto, proto_cmd_req_t *req);
extern int proto_cmd_request_events(proto_handle_t *proto, UWORD *req_events);

#endif
