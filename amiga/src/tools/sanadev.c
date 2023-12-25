#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include <devices/sana2.h>
#include <devices/plipbox.h>

#include "compiler.h"
#include "sanadev.h"

struct port_req
{
  struct MsgPort *port;
  struct IOSana2Req *req;
};
typedef struct port_req port_req_t;

/* private handle struct */
struct sanadev_handle
{
  struct Device *sana_dev;
  port_req_t cmd_pr;
  port_req_t event_pr;
};

/* copy helper for SANA-II device */
static ASM SAVEDS int MemCopy(REG(a0, UBYTE *to),
                              REG(a1, UBYTE *from),
                              REG(d0, LONG len))
{
  CopyMem(from, to, len);
  return 1;
}

static const ULONG sana_tags[] = {
    S2_CopyToBuff, (ULONG)MemCopy,
    S2_CopyFromBuff, (ULONG)MemCopy,
    TAG_DONE, 0};

static BOOL alloc_port_req(port_req_t *pr, UWORD *error)
{
  /* create read port */
  pr->port = CreateMsgPort();
  if (pr->port == NULL)
  {
    if (error != NULL)
    {
      *error = SANADEV_ERROR_NO_PORT;
    }
    return FALSE;
  }

  /* create IO request */
  pr->req = (struct IOSana2Req *)CreateIORequest(pr->port, sizeof(struct IOSana2Req));
  if (pr->req == NULL)
  {
    if (error != NULL)
    {
      *error = SANADEV_ERROR_NO_IOREQ;
    }
    DeleteMsgPort(pr->port);
    pr->port = NULL;
    return FALSE;
  }
  return TRUE;
}

static void free_port_req(port_req_t *pr)
{
  if (pr->port != NULL)
  {
    DeleteMsgPort(pr->port);
    pr->port = NULL;
  }

  if (pr->req != NULL)
  {
    DeleteIORequest(pr->req);
    pr->req = NULL;
  }
}

static void clone_req(const port_req_t *src, port_req_t *dst)
{
  CopyMem(src->req, dst->req, sizeof(struct IOSana2Req));
  // restore reply port for req
  dst->req->ios2_Req.io_Message.mn_ReplyPort = dst->port;
}

static ULONG get_port_req_mask(port_req_t *pr)
{
  if (pr->port == NULL)
  {
    return 0;
  }
  return 1UL << pr->port->mp_SigBit;
}

static struct IOSana2Req *get_port_req_next_req(port_req_t *pr)
{
  if (pr == NULL)
  {
    return NULL;
  }
  return (struct IOSana2Req *)GetMsg(pr->port);
}

// ----- open/close -----

sanadev_handle_t *sanadev_open(const char *name, ULONG unit, ULONG flags, UWORD *error)
{
  sanadev_handle_t *sh;

  sh = AllocMem(sizeof(struct sanadev_handle), MEMF_ANY | MEMF_CLEAR);
  if (sh == NULL)
  {
    return NULL;
  }
  *error = SANADEV_OK;

  BOOL ok = alloc_port_req(&sh->cmd_pr, error);
  if (!ok)
  {
    goto fail;
  }

  /* store copy buffer pointers */
  sh->cmd_pr.req->ios2_BufferManagement = sana_tags;

  /* open device */
  if (OpenDevice((STRPTR)name, unit, (struct IORequest *)sh->cmd_pr.req, flags) != 0)
  {
    *error = SANADEV_ERROR_OPEN_DEVICE;
    goto fail;
  }

  /* fetch device */
  sh->sana_dev = sh->cmd_pr.req->ios2_Req.io_Device;

  /* done. return handle */
  return sh;

fail:
  sanadev_close(sh);
  return NULL;
}

void sanadev_close(sanadev_handle_t *sh)
{
  if (sh->sana_dev != NULL)
  {
    CloseDevice((struct IORequest *)sh->cmd_pr.req);
  }

  free_port_req(&sh->cmd_pr);

  FreeMem(sh, sizeof(struct sanadev_handle));
}

// ----- events -----

BOOL sanadev_event_init(sanadev_handle_t *sh, UWORD *error)
{
  BOOL ok = alloc_port_req(&sh->event_pr, error);
  if (!ok)
  {
    return FALSE;
  }

  clone_req(&sh->cmd_pr, &sh->event_pr);
  return TRUE;
}

void sanadev_event_exit(sanadev_handle_t *sh)
{
  free_port_req(&sh->event_pr);
}

void sanadev_event_start(sanadev_handle_t *sh, ULONG event_mask)
{
  sh->event_pr.req->ios2_WireError = event_mask;
  sh->event_pr.req->ios2_Req.io_Command = S2_ONEVENT;
  SendIO((struct IORequest *)sh->event_pr.req);
}

void sanadev_event_stop(sanadev_handle_t *sh)
{
  struct IORequest *req = (struct IORequest *)sh->event_pr.req;

  if (!CheckIO(req))
  {
    AbortIO(req);
  }
  WaitIO(req);
}

ULONG sanadev_event_get_mask(sanadev_handle_t *sh)
{
  return get_port_req_mask(&sh->event_pr);
}

BOOL sanadev_event_get_event(sanadev_handle_t *sh, ULONG *event_mask)
{
  struct IOSana2Req *req = get_port_req_next_req(&sh->event_pr);
  if (req != NULL)
  {
    *event_mask = sh->event_pr.req->ios2_WireError;
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

BOOL sanadev_event_wait(sanadev_handle_t *sh, ULONG *event_mask)
{
  WaitPort(sh->event_pr.port);
  return sanadev_event_get_event(sh, event_mask);
}

// ----- Commands -----

struct IOSana2Req *sanadev_cmd_req(sanadev_handle_t *sh)
{
  return sh->cmd_pr.req;
}

BOOL sanadev_cmd(sanadev_handle_t *sh, UWORD cmd)
{
  struct IOSana2Req *sana_req = sh->cmd_pr.req;
  sana_req->ios2_Req.io_Command = cmd;

  if (DoIO((struct IORequest *)sana_req) != 0)
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

BOOL sanadev_cmd_online(sanadev_handle_t *sh)
{
  return sanadev_cmd(sh, S2_ONLINE);
}

BOOL sanadev_cmd_offline(sanadev_handle_t *sh)
{
  return sanadev_cmd(sh, S2_OFFLINE);
}

BOOL sanadev_cmd_get_station_address(sanadev_handle_t *sh, sanadev_mac_t cur_mac, sanadev_mac_t def_mac)
{
  BOOL ok = sanadev_cmd(sh, S2_GETSTATIONADDRESS);
  if (ok)
  {
    CopyMem(sh->cmd_pr.req->ios2_SrcAddr, cur_mac, SANADEV_MAC_SIZE);
    CopyMem(sh->cmd_pr.req->ios2_DstAddr, def_mac, SANADEV_MAC_SIZE);
  }
  return ok;
}

// ----- misc -----

static void get_error(struct IOSana2Req *sana_req, UWORD *error, UWORD *wire_error)
{
  *error = sana_req->ios2_Req.io_Error;
  *wire_error = sana_req->ios2_WireError;
}

static void print_error(struct IOSana2Req *sana_req)
{
  Printf((STRPTR) "SANA-II IO failed: cmd=%04lx -> error=%ld, wire_error=%ld\n",
         (ULONG)sana_req->ios2_Req.io_Command,
         (ULONG)sana_req->ios2_Req.io_Error,
         (ULONG)sana_req->ios2_WireError);
}

void sanadev_cmd_get_error(sanadev_handle_t *sh, UWORD *error, UWORD *wire_error)
{
  get_error(sh->cmd_pr.req, error, wire_error);
}

void sanadev_cmd_print_error(sanadev_handle_t *sh)
{
  print_error(sh->cmd_pr.req);
}

void sanadev_print_mac(sanadev_mac_t mac)
{
  Printf("%02lx:%02lx:%02lx:%02lx:%02lx:%02lx",
         (ULONG)mac[0], (ULONG)mac[1], (ULONG)mac[2],
         (ULONG)mac[3], (ULONG)mac[4], (ULONG)mac[5]);
}
