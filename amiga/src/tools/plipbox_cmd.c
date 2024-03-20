#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include <devices/sana2.h>
#include <devices/plipbox.h>

#include "compiler.h"
#include "sanadev.h"
#include "plipbox_cmd.h"
#include "req_shared.h"

/* special plipbox commands */
BOOL plipbox_cmd_get_version(sanadev_handle_t *sh, UWORD *dev_version, UWORD *fw_version)
{
  BOOL ok = sanadev_cmd(sh, S2PB_GET_VERSION);
  if (ok)
  {
    struct IOSana2Req *req = sanadev_cmd_req(sh);
    ULONG ver = req->ios2_WireError;
    *dev_version = (UWORD)(ver >> 16);
    *fw_version = (UWORD)(ver & 0xffff);
  }
  return ok;
}

int plipbox_cmd_do_request(sanadev_handle_t *sh, s2pb_request_t *req)
{
  struct IOSana2Req *sana_req = sanadev_cmd_req(sh);
  sana_req->ios2_DataLength = sizeof(s2pb_request_t);
  sana_req->ios2_Data = req;
  BOOL ok = sanadev_cmd(sh, S2PB_DO_REQUEST);
  if(!ok) {
    return REQ_ERROR_SANA2;
  } else {
    return req->status;
  }
}
