#ifndef PROTO_CMD_H
#define PROTO_CMD_H

#include "proto_atom.h"

typedef UBYTE mac_t[6];
#define MAC_SIZE 6

#define PROTO_RET_RX_TOO_LARGE    (PROTO_RET_CUSTOM + 0)
#define PROTO_RET_RX_ERROR        (PROTO_RET_CUSTOM + 1)
#define PROTO_RET_TX_ERROR        (PROTO_RET_CUSTOM + 2)

struct proto_param_def {
  UBYTE index;
  UBYTE type;
  UBYTE format;
  UWORD size;
  ULONG tag;
};
typedef struct proto_param_def proto_param_def_t;

extern int proto_cmd_init(proto_handle_t *proto, UWORD token);
extern int proto_cmd_ping(proto_handle_t *proto, UWORD *token);
extern int proto_cmd_exit(proto_handle_t *proto);

extern int proto_cmd_attach(proto_handle_t *proto);
extern int proto_cmd_detach(proto_handle_t *proto);

extern int proto_cmd_get_status(proto_handle_t *proto, UWORD *status);

extern int proto_cmd_send_frame(proto_handle_t *proto, UBYTE *buf, UWORD num_bytes, UWORD *status);
extern int proto_cmd_recv_frame(proto_handle_t *proto, UBYTE *buf, UWORD max_bytes, UWORD *num_bytes, UWORD *status);

extern int proto_cmd_get_version(proto_handle_t *proto, UWORD *version);
extern int proto_cmd_get_cur_mac(proto_handle_t *proto, mac_t mac);
extern int proto_cmd_get_def_mac(proto_handle_t *proto, mac_t mac);
extern int proto_cmd_set_cur_mac(proto_handle_t *proto, mac_t mac);

/* ----- param ----- */

extern int proto_cmd_param_get_num(proto_handle_t *proto, UWORD *num_param);
extern int proto_cmd_param_find_tag(proto_handle_t *proto, ULONG tag, UWORD *id);
extern int proto_cmd_param_get_def(proto_handle_t *proto, UWORD id, proto_param_def_t *def);
extern int proto_cmd_param_get_val(proto_handle_t *proto, UWORD id, UWORD size, UBYTE *data);
extern int proto_cmd_param_set_val(proto_handle_t *proto, UWORD id, UWORD size, UBYTE *data);

/* ----- prefs ----- */

extern int proto_cmd_prefs_reset(proto_handle_t *proto);
extern int proto_cmd_prefs_load(proto_handle_t *proto, UWORD *status);
extern int proto_cmd_prefs_save(proto_handle_t *proto, UWORD *status);

#endif
