#include <string.h>

#include "types.h"

#ifdef DEBUG_REQ
#define DEBUG
#endif

#include "debug.h"
#include "proto_cmd.h"
#include "req.h"
#include "req_shared.h"
#include "param_shared.h"
#include "req_param.h"

static u08 in_buf[REQ_IN_BUF_SIZE];
static u08 out_buf[REQ_OUT_BUF_SIZE];

static void dispatch_req(proto_cmd_req_t *req)
{
  DS("dispatch_req:"); DB(req->command); DNL;
  switch(req->command) {

  // ----- param commands -----
  // GET_NUM: in:- out_extra:(u08)total_params
  case REQ_PARAM_GET_NUM:
    req_param_get_num(req);
    break;
  // FIND_TAG: in:(u32)tag  out_extra:param_index
  case REQ_PARAM_FIND_TAG:
    req_param_find_tag(req);
    break;
  // GET_DEF: in_extra:param_index out:param_def
  case REQ_PARAM_GET_DEF:
    req_param_get_def(req);
    break;
  // GET_VAL: in_extra:param_index out:param_data
  case REQ_PARAM_GET_VAL:
    req_param_get_val(req);
    break;
  // SET_VAL: in_extra:param_index in:param_data
  case REQ_PARAM_SET_VAL:
    req_param_set_val(req);
    break;
  // RESET: in:- out:-
  case REQ_PARAM_RESET:
    req_param_reset(req);
    break;
  // LOAD: in:- out:-
  case REQ_PARAM_LOAD:
    req_param_load(req);
    break;
  // SAVE: in:- out:-
  case REQ_PARAM_SAVE:
    req_param_save(req);
    break;
  // GET_DEF_MAC: out:mac
  case REQ_PARAM_GET_DEF_MAC:
    req_param_get_def_mac(req);
    break;
  // MAC_GET_CUR: out:mac
  case REQ_PARAM_GET_CUR_MAC:
    req_param_get_cur_mac(req);
    break;
  // MAC_SET_CUR: in:mac
  case REQ_PARAM_SET_CUR_MAC:
    req_param_set_cur_mac(req);
    break;

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
