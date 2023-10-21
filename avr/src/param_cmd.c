/*
  param_cmd.c - implement the proto_cmd_api for parameter commands
*/

#include "global.h"

#ifdef DEBUG_PARAM
#define DEBUG
#endif

#include "debug.h"
#include "param.h"
#include "proto_cmd.h"

void proto_cmd_api_set_mode(u16 new_mode)
{
  param_set_mode(new_mode);
}

u16 proto_cmd_api_get_mode(void)
{
  return param_get_mode();
}

void proto_cmd_api_set_mac(mac_t mac)
{
  param_set_mac(mac);
}

void proto_cmd_api_get_mac(mac_t mac)
{
  param_get_mac(mac);
}

void proto_cmd_api_get_def_mac(mac_t mac)
{
  param_get_def_mac(mac);
}

void proto_cmd_api_reset_prefs(void)
{
  param_reset();
}

u16  proto_cmd_api_load_prefs(void)
{
  return param_load();
}

u16  proto_cmd_api_save_prefs(void)
{
  return param_save();
}
