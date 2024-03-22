#ifndef PROTO_REQ
#define PROTO_REQ

extern int proto_req_get_cur_mac(proto_handle_t *proto, mac_t mac);
extern int proto_req_get_def_mac(proto_handle_t *proto, mac_t mac);
extern int proto_req_set_cur_mac(proto_handle_t *proto, mac_t mac);

#endif
