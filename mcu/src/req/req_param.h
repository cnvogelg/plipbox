#ifndef REQ_PARAM_H
#define REQ_PARAM_H

#include "proto_api.h"

extern void req_param_get_num(proto_api_req_t *req);
extern void req_param_find_tag(proto_api_req_t *req);
extern void req_param_get_def(proto_api_req_t *req);
extern void req_param_get_val(proto_api_req_t *req);
extern void req_param_set_val(proto_api_req_t *req);

extern void req_param_reset(proto_api_req_t *req);
extern void req_param_load(proto_api_req_t *req);
extern void req_param_save(proto_api_req_t *req);

extern void req_param_get_def_mac(proto_api_req_t *req);
extern void req_param_get_cur_mac(proto_api_req_t *req);
extern void req_param_set_cur_mac(proto_api_req_t *req);

#endif
