#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "compiler.h"
#include "sanadev.h"
#include "plipbox_req_mode.h"
#include "plipbox_cmd.h"
#include "mode.h"
#include "req_shared.h"

// ----- mode -----

int plipbox_req_mode_get_num(sanadev_handle_t *sh, UBYTE *num)
{
  s2pb_request_t req = {
    .command = REQ_MODE_GET_NUM,
    .in_size = 0,
    .in_data = NULL,
    .out_size = sizeof(UWORD),
    .out_data = num
  };

  int res = plipbox_cmd_do_request(sh, &req);
  *num = req.out_extra;
  return res;
}

int plipbox_req_mode_find_tag(sanadev_handle_t *sh, ULONG tag, UBYTE *id)
{
  s2pb_request_t req = {
    .command = REQ_MODE_FIND_TAG,
    .in_size = sizeof(ULONG),
    .in_data = &tag,
    .out_size = 0,
    .out_data = NULL
  };

  int res = plipbox_cmd_do_request(sh, &req);
  *id = req.out_extra;
  return res;
}

int plipbox_req_mode_get_def(sanadev_handle_t *sh, UBYTE id, mode_def_t *def)
{
  s2pb_request_t req = {
    .command = REQ_MODE_GET_DEF,
    .in_extra = id,
    .in_size = 0,
    .in_data = NULL,
    .out_size = sizeof(mode_def_t),
    .out_data = def
  };

  return plipbox_cmd_do_request(sh, &req);
}
