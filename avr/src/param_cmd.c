/*
  param_cmd.c - implement the proto_cmd_api for parameter commands
*/

#include "types.h"

#ifdef DEBUG_PARAM
#define DEBUG
#endif

#include "debug.h"
#include "param.h"
#include "proto_cmd.h"
#include "proto_cmd_shared.h"
#include "param_shared.h"
#include "wire.h"

static u08 param_id = 0;
static u08 param_def_buf[PARAM_DEF_SIZE];

// ----- MAC -----

void proto_cmd_api_get_cur_mac(mac_t mac)
{
  param_get_cur_mac(mac);
}

void proto_cmd_api_get_def_mac(mac_t mac)
{
  param_get_def_mac(mac);
}

// ----- param -----

u08 proto_cmd_api_param_get_num(void)
{
  return param_get_num();
}

void proto_cmd_api_param_find_tag(u32 tag)
{
  param_id = param_find_tag(tag);
}

u08  proto_cmd_api_param_get_id(void)
{
  return param_id;
}

void proto_cmd_api_param_set_id(u08 id)
{
  if(id < param_get_num()) {
    param_id = id;
  } else {
    param_id = 0;
  }
}

u08 *proto_cmd_api_param_get_def(u16 *size)
{
  param_def_t def;
  param_get_def(param_id, &def);

  // convert param_def_t to wire format
  // param description
  // +00 u08 index
  // +01 u08 type
  // +02 u08 format
  // +03 u08 reserved
  // +04 u16 size
  // +06 u32 tag
  // =10

  u08 *p = param_def_buf;
  p[0] = def.index;
  p[1] = def.type;
  p[2] = def.format;
  p[3] = 0;
  wire_h2w_u16(def.size, &p[4]);
  wire_h2w_u32(def.tag, &p[6]);

  *size = PARAM_DEF_SIZE;
  return param_def_buf;
}

u08 *proto_cmd_api_param_get_val(u16 *size)
{
  param_def_t def;
  param_get_def(param_id, &def);

  *size = def.size;
  u08 *buf = param_get_data(param_id);

#ifdef DEBUG
  DS("data[");
  for(u16 i=0;i<*size;i++) {
    DB(buf[i]);
  }
  DC(']');
  DNL;
#endif

  return buf;
}

void proto_cmd_api_param_set_val(const u08 *buf, u16 size)
{
#ifdef DEBUG
  DS("data[");
  for(u16 i=0;i<size;i++) {
    DB(buf[i]);
  }
  DC(']');
  DNL;
#endif
}

void proto_cmd_api_prefs_reset(void)
{
  param_reset();
}

u16  proto_cmd_api_prefs_load(void)
{
  return param_load();
}

u16  proto_cmd_api_prefs_save(void)
{
  return param_save();
}
