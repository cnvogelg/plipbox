#include <string.h>

#include "types.h"

#ifdef DEBUG_REQ
#define DEBUG
#endif

#include "debug.h"
#include "nic.h"
#include "param_shared.h"
#include "wire.h"
#include "req_nic.h"
#include "req_shared.h"

void req_nic_get_num(proto_api_req_t *req)
{
  req->out_extra = nic_get_num_nics();
  DS("NIC_GET_NUM:"); DB(req->out_extra); DNL;
}

void req_nic_find_tag(proto_api_req_t *req)
{
  u32 tag = 0;
  wire_w2h_u32(req->in_buf, &tag);
  u08 index = nic_find_tag(tag);
  DS("NIC_FIND_TAG:"); DL(tag); DC(':'); DB(index); DNL;
  req->out_extra = index;
}

void req_nic_get_def(proto_api_req_t *req)
{
  u08 index = req->in_extra;
  nic_def_t def;
  nic_get_def(index, &def);
  DS("NIC_GET_DEF"); DB(index); DNL;
  // convert mode_def_t to wire format
  // param description
  // +00 u32 tag
  // +04 u16 caps
  // =06
  u08 *p = req->out_buf;
  wire_h2w_u32(def.tag, &p[0]);
  wire_h2w_u16(def.caps, &p[4]);
  req->out_size = NIC_DEF_SIZE;
}
