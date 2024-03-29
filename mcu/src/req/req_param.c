#include <string.h>

#include "types.h"

#ifdef DEBUG_REQ
#define DEBUG
#endif

#include "debug.h"
#include "param.h"
#include "param_shared.h"
#include "wire.h"
#include "req_param.h"
#include "req_shared.h"

void req_param_get_num(proto_cmd_req_t *req)
{
  req->out_extra = param_get_num();
  DS("PARAM_GET_NUM:"); DB(req->out_extra); DNL;
}

void req_param_find_tag(proto_cmd_req_t *req)
{
  u32 tag = 0;
  wire_w2h_u32(req->in_buf, &tag);
  u08 index = param_find_tag(tag);
  DS("PARAM_FIND_TAG:"); DL(tag); DC(':'); DB(index); DNL;
  req->out_extra = index;
}

void req_param_get_def(proto_cmd_req_t *req)
{
  u08 index = req->in_extra;
  param_def_t def;
  param_get_def(index, &def);
  DS("PARAM_GET_DEF"); DB(index); DNL;
  // convert param_def_t to wire format
  // param description
  // +00 u08 index
  // +01 u08 type
  // +02 u08 format
  // +03 u08 reserved
  // +04 u16 size
  // +06 u32 tag
  // =10
  u08 *p = req->out_buf;
  p[0] = index;
  p[1] = def.type;
  p[2] = def.format;
  p[3] = 0;
  wire_h2w_u16(def.size, &p[4]);
  wire_h2w_u32(def.tag, &p[6]);
  req->out_size = PARAM_DEF_SIZE;
}

void req_param_get_val(proto_cmd_req_t *req)
{
  u08 index = req->in_extra;
  DS("PARAM_GET_VAL:"); DB(index);
  u08 *data = param_get_data(index);
  if(data != NULL) {
    req->out_buf = data;
    req->out_size = param_get_size(index);
#ifdef DEBUG
    DS("data[");
    for(u16 i=0;i<*size;i++) {
      DB(data[i]);
    }
    DC(']');
    DNL;
#endif
  } else {
    req->status = REQ_ERROR_LOADING_DATA;
    DS("->ERROR!");
  }
  DNL;
}

void req_param_set_val(proto_cmd_req_t *req)
{
  u08 index = req->in_extra;
  DS("PARAM_SET_VAL:"); DB(index);
  u08 *data = param_get_data(index);
  u16 size = param_get_size(index);
  if((data != NULL) && (size == req->in_size)) {
    memcpy(data, req->in_buf, size);
#ifdef DEBUG
    DS("data[");
    for(u16 i=0;i<size;i++) {
      DB(data[i]);
    }
    DC(']');
    DNL;
#endif
  } else {
    req->status = REQ_ERROR_SAVING_DATA;
    DS("->ERROR!");
  }
  DNL;
}

void req_param_reset(proto_cmd_req_t *req)
{
  DS("PREFS_RESET"); DNL;
  param_reset();
}

void req_param_load(proto_cmd_req_t *req)
{
  u08 res = param_load();
  DS("PREFS_LOAD:"); DB(res); DNL;
  if(res != 0) {
    req->status = REQ_ERROR_LOADING_DATA;
  }
}

void req_param_save(proto_cmd_req_t *req)
{
  u08 res = param_save();
  DS("PREFS_SAVE:"); DB(res); DNL;
  if(res != 0) {
    req->status = REQ_ERROR_SAVING_DATA;
  }
}

void req_param_get_def_mac(proto_cmd_req_t *req)
{
  mac_t mac;
  param_get_def_mac(mac);
  DS("MAC_GET_DEF:"); DM(mac); DNL;
  memcpy(req->out_buf, mac, MAC_SIZE);
  req->out_size = MAC_SIZE;
}

void req_param_get_cur_mac(proto_cmd_req_t *req)
{
  mac_t mac;
  param_get_cur_mac(mac);
  DS("MAC_GET_CUR:"); DM(mac); DNL;
  memcpy(req->out_buf, mac, MAC_SIZE);
  req->out_size = MAC_SIZE;
}

void req_param_set_cur_mac(proto_cmd_req_t *req)
{
  DS("MAC_SET_CUR:"); DNL;
  if(req->in_size != MAC_SIZE) {
    DS("SIZE?");
    req->status = REQ_ERROR_WRONG_IN_SIZE;
  } else {
    mac_t mac;
    memcpy(mac, req->in_buf, MAC_SIZE);
    DM(mac);
    param_set_cur_mac(mac);
  }
  DNL;
}

