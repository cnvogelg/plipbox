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
typedef struct proto_cmd_req proto_api_req_t;

extern int proto_cmd_reset(proto_handle_t *proto);
extern int proto_cmd_alive(proto_handle_t *proto);

extern int proto_cmd_init(proto_handle_t *proto, UWORD token);
extern int proto_cmd_ping(proto_handle_t *proto, UWORD *token);
extern int proto_cmd_exit(proto_handle_t *proto);
extern int proto_cmd_get_version(proto_handle_t *proto, UWORD *version);

extern int proto_cmd_mode_attach(proto_handle_t *proto, UWORD *mode_status);
extern int proto_cmd_mode_detach(proto_handle_t *proto, UWORD *mode_status);
extern int proto_cmd_mode_set(proto_handle_t *proto, UWORD mode);
extern int proto_cmd_mode_get(proto_handle_t *proto, UWORD *mode);

extern int proto_cmd_event_mask(proto_handle_t *proto, UWORD *event_mask);
extern int proto_cmd_mode_status(proto_handle_t *proto, UWORD *mode_status);
extern int proto_cmd_nic_status(proto_handle_t *proto, UWORD *nic_status);
extern int proto_cmd_link_status(proto_handle_t *proto, UWORD *link_status);

extern int proto_cmd_send_frame(proto_handle_t *proto, UBYTE *buf, UWORD num_bytes, UWORD *ret_event_mask);
extern int proto_cmd_recv_frame(proto_handle_t *proto, UBYTE *buf, UWORD max_bytes, UWORD *num_bytes, UWORD *ret_event_mask);

extern int proto_cmd_rx_error(proto_handle_t *proto, UWORD *rx_error);
extern int proto_cmd_tx_error(proto_handle_t *proto, UWORD *tx_error);

extern int proto_cmd_rx_drop_count(proto_handle_t *proto, UWORD *rx_drops);
extern int proto_cmd_tx_drop_count(proto_handle_t *proto, UWORD *tx_drops);

extern int proto_cmd_request(proto_handle_t *proto, proto_api_req_t *req);
extern int proto_cmd_request_event_mask(proto_handle_t *proto, UWORD *event_mask);

#endif
