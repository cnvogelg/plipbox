#include <string.h>

#include "types.h"

#ifdef DEBUG_REQ
#define DEBUG
#endif

#include "debug.h"
#include "mode.h"
#include "param_shared.h"
#include "wire.h"
#include "req_mode.h"
#include "req_shared.h"

void req_mode_get_num(proto_api_req_t *req)
{
  req->out_extra = mode_get_num_modes();
  DS("MODE_GET_NUM:"); DB(req->out_extra); DNL;
}

void req_mode_find_tag(proto_api_req_t *req)
{
  u32 tag = 0;
  wire_w2h_u32(req->in_buf, &tag);
  u08 index = mode_find_tag(tag);
  DS("MODE_FIND_TAG:"); DL(tag); DC(':'); DB(index); DNL;
  req->out_extra = index;
}

void req_mode_get_def(proto_api_req_t *req)
{
  u08 index = req->in_extra;
  mode_def_t def;
  mode_get_def(index, &def);
  DS("MODE_GET_DEF"); DB(index); DNL;
  // convert mode_def_t to wire format
  // param description
  // +00 u32 tag
  // =04
  u08 *p = req->out_buf;
  wire_h2w_u32(def.tag, &p[0]);
  req->out_size = MODE_DEF_SIZE;
}
