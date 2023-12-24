#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include <devices/sana2.h>
#include <devices/plipbox.h>

#include "compiler.h"
#include "sanadev.h"
#include "plipbox_cmd.h"

/* special plipbox commands */
BOOL plipbox_cmd_get_version(sanadev_handle_t *sh, UWORD *dev_version, UWORD *fw_version)
{
  BOOL ok = sanadev_cmd(sh, S2PB_GET_VERSION);
  if(ok) {
    struct IOSana2Req *req = sanadev_cmd_req(sh);
    ULONG ver = req->ios2_WireError;
    *dev_version = (UWORD)(ver >> 16);
    *fw_version = (UWORD)(ver & 0xffff);
  }
  return ok;
}

// ----- param -----

BOOL plipbox_cmd_param_get_num(sanadev_handle_t *sh, UWORD *num)
{
  BOOL ok = sanadev_cmd(sh, S2PB_PARAM_GET_NUM);
  if(ok) {
    struct IOSana2Req *req = sanadev_cmd_req(sh);
    *num = (UWORD)req->ios2_WireError;
  } else {
    *num = 0;
  }
  return ok;
}

BOOL plipbox_cmd_param_find_tag(sanadev_handle_t *sh, ULONG tag, UWORD *id)
{
  struct IOSana2Req *req = sanadev_cmd_req(sh);
  req->ios2_WireError = tag;
  BOOL ok = sanadev_cmd(sh, S2PB_PARAM_FIND_TAG);
  if(ok) {
    *id = (UWORD)req->ios2_WireError;
  } else {
    *id = S2PB_NO_INDEX;
  }
  return ok;
}

BOOL plipbox_cmd_param_get_def(sanadev_handle_t *sh, UWORD id, s2pb_param_def_t *def)
{
  struct IOSana2Req *req = sanadev_cmd_req(sh);
  req->ios2_WireError = id;
  req->ios2_DataLength = sizeof(s2pb_param_def_t);
  req->ios2_Data = def;
  return sanadev_cmd(sh, S2PB_PARAM_GET_DEF);
}

BOOL plipbox_cmd_param_get_val(sanadev_handle_t *sh, UWORD id, UWORD size, UBYTE *data)
{
  struct IOSana2Req *req = sanadev_cmd_req(sh);
  req->ios2_WireError = id;
  req->ios2_DataLength = size;
  req->ios2_Data = data;
  return sanadev_cmd(sh, S2PB_PARAM_GET_VAL);
}

BOOL plipbox_cmd_param_set_val(sanadev_handle_t *sh, UWORD id, UWORD size, UBYTE *data)
{
  struct IOSana2Req *req = sanadev_cmd_req(sh);
  req->ios2_WireError = id;
  req->ios2_DataLength = size;
  req->ios2_Data = data;
  return sanadev_cmd(sh, S2PB_PARAM_SET_VAL);
}

// ----- prefs -----

BOOL plipbox_cmd_prefs_reset(sanadev_handle_t *sh)
{
  return sanadev_cmd(sh, S2PB_PREFS_RESET);
}

BOOL plipbox_cmd_prefs_load(sanadev_handle_t *sh, UWORD *status)
{
  BOOL ok = sanadev_cmd(sh, S2PB_PREFS_LOAD);
  if(ok) {
    struct IOSana2Req *req = sanadev_cmd_req(sh);
    *status = req->ios2_WireError;
  }
  return TRUE;
}

BOOL plipbox_cmd_prefs_save(sanadev_handle_t *sh, UWORD *status)
{
  BOOL ok = sanadev_cmd(sh, S2PB_PREFS_SAVE);
  if(ok) {
    struct IOSana2Req *req = sanadev_cmd_req(sh);
    *status = req->ios2_WireError;
  }
  return TRUE;
}
