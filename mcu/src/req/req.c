#include <string.h>

#include "types.h"

#ifdef DEBUG_PROTO_REQ
#define DEBUG
#endif

#include "debug.h"
#include "proto_cmd.h"
#include "req.h"
#include "req_shared.h"
#include "param.h"
#include "wire.h"
#include "param_shared.h"

static u08 in_buf[REQ_IN_BUF_SIZE];
static u08 out_buf[REQ_OUT_BUF_SIZE];

static void dispatch_req(proto_cmd_req_t *req)
{
  DS("dispatch_req:"); DB(req->command); DNL;
  switch(req->command) {

  // ----- param commands -----
  // GET_NUM: in:- out_extra:(u08)total_params
  case REQ_PARAM_GET_NUM:
    req->out_extra = param_get_num();
    DS("PARAM_GET_NUM:"); DB(req->out_extra); DNL;
    break;

  // FIND_TAG: in:(u32)tag  out_extra:param_index
  case REQ_PARAM_FIND_TAG: {
    u32 tag = 0;
    wire_w2h_u32(req->in_buf, &tag);
    u08 index = param_find_tag(tag);
    DS("PARAM_FIND_TAG:"); DL(tag); DC(':'); DB(index); DNL;
    req->out_extra = index;
    break;
  }

  // GET_DEF: in_extra:param_index out:param_def
  case REQ_PARAM_GET_DEF: {
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
    p[0] = def.index;
    p[1] = def.type;
    p[2] = def.format;
    p[3] = 0;
    wire_h2w_u16(def.size, &p[4]);
    wire_h2w_u32(def.tag, &p[6]);
    req->out_size = PARAM_DEF_SIZE;
    break;
  }

  // GET_VAL: in_extra:param_index out:param_data
  case REQ_PARAM_GET_VAL: {
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
    break;
  }

  // SET_VAL: in_extra:param_index in:param_data
  case REQ_PARAM_SET_VAL: {
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
    break;
  }

  // ----- prefs commands -----
  // PREFS_RESET: in:- out:-
  case REQ_PREFS_RESET: {
    DS("PREFS_RESET"); DNL;
    param_reset();
    break;
  }

  // PREFS_LOAD: in:- out:-
  case REQ_PREFS_LOAD: {
    u08 res = param_load();
    DS("PREFS_LOAD:"); DB(res); DNL;
    if(res != 0) {
      req->status = REQ_ERROR_LOADING_DATA;
    }
    break;
  }

  // PREFS_SAVE: in:- out:-
  case REQ_PREFS_SAVE: {
    u08 res = param_save();
    DS("PREFS_SAVE:"); DB(res); DNL;
    if(res != 0) {
      req->status = REQ_ERROR_SAVING_DATA;
    }
    break;
  }

  // ----- MAC commands -----
  // MAC_GET_DEF: out:mac
  case REQ_MAC_GET_DEF: {
    mac_t mac;
    param_get_def_mac(mac);
    DS("MAC_GET_DEF:"); DM(mac); DNL;
    memcpy(req->out_buf, mac, MAC_SIZE);
    req->out_size = MAC_SIZE;
    break;
  }

  // MAC_GET_CUR: out:mac
  case REQ_MAC_GET_CUR: {
    mac_t mac;
    param_get_cur_mac(mac);
    DS("MAC_GET_CUR:"); DM(mac); DNL;
    memcpy(req->out_buf, mac, MAC_SIZE);
    req->out_size = MAC_SIZE;
    break;
  }

  // MAC_SET_CUR: in:mac
  case REQ_MAC_SET_CUR: {
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

  // unknown command
  default:
    DS("UNKNOWN COMMAND!"); DNL;
    req->status = REQ_ERROR_UNKNOWN_COMMAND;
    break;
  }
}

void proto_cmd_api_req_in(proto_cmd_req_t *req)
{
  // check in_size
  if(req->in_size > REQ_IN_BUF_SIZE) {
    req->status = REQ_ERROR_IN_TOO_LARGE;
  } else {
    req->status = REQ_OK;
  }

  req->in_buf = in_buf;
}

void proto_cmd_api_req_out(proto_cmd_req_t *req)
{
  req->out_buf = out_buf;
  req->out_size = 0;
  req->out_extra = 0;

  // no error in req_in?
  if(req->status == REQ_OK) {
    dispatch_req(req);
  }
}
