#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "compiler.h"
#include "sanadev.h"
#include "plipbox_req.h"
#include "plipbox_cmd.h"
#include "param.h"
#include "req_shared.h"

// ----- param -----

int plipbox_req_param_get_num(sanadev_handle_t *sh, UBYTE *num)
{
  s2pb_request_t req = {
    .command = REQ_PARAM_GET_NUM,
    .in_size = 0,
    .in_data = NULL,
    .out_size = sizeof(UWORD),
    .out_data = num
  };

  int res = plipbox_cmd_do_request(sh, &req);
  *num = req.out_extra;
  return res;
}

int plipbox_req_param_find_tag(sanadev_handle_t *sh, ULONG tag, UBYTE *id)
{
  s2pb_request_t req = {
    .command = REQ_PARAM_FIND_TAG,
    .in_size = sizeof(ULONG),
    .in_data = &tag,
    .out_size = 0,
    .out_data = NULL
  };

  int res = plipbox_cmd_do_request(sh, &req);
  *id = req.out_extra;
  return res;
}

int plipbox_req_param_get_def(sanadev_handle_t *sh, UBYTE id, param_def_t *def)
{
  s2pb_request_t req = {
    .command = REQ_PARAM_GET_DEF,
    .in_extra = id,
    .in_size = 0,
    .in_data = NULL,
    .out_size = sizeof(param_def_t),
    .out_data = def
  };

  return plipbox_cmd_do_request(sh, &req);
}

int plipbox_req_param_get_val(sanadev_handle_t *sh, UBYTE id, UWORD size, UBYTE *data)
{
  s2pb_request_t req = {
    .command = REQ_PARAM_GET_VAL,
    .in_extra = id,
    .in_size = 0,
    .in_data = NULL,
    .out_size = size,
    .out_data = data
  };

  return plipbox_cmd_do_request(sh, &req);
}

int plipbox_req_param_set_val(sanadev_handle_t *sh, UBYTE id, UWORD size, UBYTE *data)
{
  s2pb_request_t req = {
    .command = REQ_PARAM_SET_VAL,
    .in_extra = id,
    .in_size = size,
    .in_data = data,
    .out_size = 0,
    .out_data = NULL
  };

  return plipbox_cmd_do_request(sh, &req);
}

int plipbox_req_param_get_word(sanadev_handle_t *sh, UBYTE id, UWORD *value)
{
  UBYTE data[2];
  BOOL ok = plipbox_req_param_get_val(sh, id, 2, data);
  if(!ok) {
    return FALSE;
  }
  *value = param_get_wire_value(data, 2);
  return TRUE;
}

int plipbox_req_param_set_word(sanadev_handle_t *sh, UBYTE id, UWORD value)
{
  UBYTE data[2];
  param_set_wire_value(data, value, 2);
  return plipbox_req_param_set_val(sh, id, 2, data);
}

// ----- prefs -----

int plipbox_req_prefs_reset(sanadev_handle_t *sh)
{
  s2pb_request_t req = {
    .command = REQ_PREFS_RESET,
    .in_size = 0,
    .in_data = NULL,
    .out_size = 0,
    .out_data = NULL
  };

  return plipbox_cmd_do_request(sh, &req);
}

int plipbox_req_prefs_load(sanadev_handle_t *sh)
{
  s2pb_request_t req = {
    .command = REQ_PREFS_LOAD,
    .in_size = 0,
    .in_data = NULL,
    .out_size = 0,
    .out_data = NULL
  };

  return plipbox_cmd_do_request(sh, &req);
}

int plipbox_req_prefs_save(sanadev_handle_t *sh)
{
  s2pb_request_t req = {
    .command = REQ_PREFS_SAVE,
    .in_size = 0,
    .in_data = NULL,
    .out_size = 0,
    .out_data = NULL
  };

  return plipbox_cmd_do_request(sh, &req);
}
