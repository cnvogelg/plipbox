#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <clib/alib_protos.h>

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

  port_req_t write_pr;
  port_req_t read_pr;
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

BOOL sanadev_event_start(sanadev_handle_t *sh, ULONG event_mask)
{
  if(sh->write_pr.req == NULL) {
    return FALSE;
  }

  sh->event_pr.req->ios2_WireError = event_mask;
  sh->event_pr.req->ios2_Req.io_Command = S2_ONEVENT;
  SendIO((struct IORequest *)sh->event_pr.req);
  return TRUE;
}

BOOL sanadev_event_stop(sanadev_handle_t *sh)
{
  struct IORequest *req = (struct IORequest *)sh->event_pr.req;
  if(req == NULL) {
    return FALSE;
  }

  if (!CheckIO(req))
  {
    AbortIO(req);
  }
  WaitIO(req);
  return TRUE;
}

ULONG sanadev_event_get_mask(sanadev_handle_t *sh)
{
  return get_port_req_mask(&sh->event_pr);
}

BOOL sanadev_event_result(sanadev_handle_t *sh, ULONG *event_mask)
{
  struct IOSana2Req *req = get_port_req_next_req(&sh->event_pr);
  if (req != NULL)
  {
    if(sh->event_pr.req->ios2_Req.io_Error != 0) {
      return FALSE;
    }
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
  if(sh->event_pr.port == NULL) {
    return FALSE;
  }

  WaitPort(sh->event_pr.port);
  return sanadev_event_result(sh, event_mask);
}

// ----- I/O -----

/* we need an own DoIO replacement since we want to pass io_Flags */
static BYTE my_do_io(struct IOSana2Req *req)
{
  BeginIO((struct IORequest *)req);
  return WaitIO((struct IORequest *)req);
}

BOOL sanadev_io_init(sanadev_handle_t *sh, UWORD *error)
{
  // write port/ioreq
  BOOL ok = alloc_port_req(&sh->write_pr, error);
  if (!ok)
  {
    return FALSE;
  }

  clone_req(&sh->cmd_pr, &sh->write_pr);

  // read port/ioreq
  ok = alloc_port_req(&sh->read_pr, error);
  if (!ok)
  {
    return FALSE;
  }

  clone_req(&sh->cmd_pr, &sh->read_pr);

  return TRUE;
}

void sanadev_io_exit(sanadev_handle_t *sh)
{
  free_port_req(&sh->write_pr);
  free_port_req(&sh->read_pr);
}

BOOL sanadev_io_write(sanadev_handle_t *sh, UWORD pkt_type, sanadev_mac_t dst_addr, APTR data, ULONG data_len)
{
  if(sh->write_pr.port == NULL) {
    return FALSE;
  }

  struct IOSana2Req *req = sh->write_pr.req;
  req->ios2_Req.io_Flags = 0;
  req->ios2_Req.io_Command = CMD_WRITE;
  req->ios2_PacketType = pkt_type;
  CopyMem(dst_addr, req->ios2_DstAddr, SANADEV_MAC_SIZE);
  req->ios2_Data = data;
  req->ios2_DataLength = data_len;

  if (my_do_io(req) != 0)
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

BOOL sanadev_io_write_raw(sanadev_handle_t *sh, APTR data, ULONG data_len)
{
  if(sh->write_pr.port == NULL) {
    return FALSE;
  }

  struct IOSana2Req *req = sh->write_pr.req;
  req->ios2_Req.io_Flags = SANA2IOF_RAW;
  req->ios2_Req.io_Command = CMD_WRITE;
  req->ios2_Data = data;
  req->ios2_DataLength = data_len;

  if (my_do_io(req) != 0)
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

BOOL sanadev_io_broadcast(sanadev_handle_t *sh, UWORD pkt_type, APTR data, ULONG data_len)
{
  if(sh->write_pr.port == NULL) {
    return FALSE;
  }

  struct IOSana2Req *req = sh->write_pr.req;
  req->ios2_Req.io_Flags = 0;
  req->ios2_Req.io_Command = S2_BROADCAST;
  req->ios2_PacketType = pkt_type;
  req->ios2_Data = data;
  req->ios2_DataLength = data_len;

  if (my_do_io(req) != 0)
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

// --- Read ---

BOOL sanadev_io_read_start(sanadev_handle_t *sh, UWORD pkt_type, APTR data, ULONG data_len, BOOL raw)
{
  if(sh->read_pr.port == NULL) {
    return FALSE;
  }

  struct IOSana2Req *req = sh->read_pr.req;
  req->ios2_PacketType = pkt_type;
  req->ios2_Req.io_Flags = raw ? SANA2IOF_RAW : 0;
  req->ios2_Req.io_Command = CMD_READ;
  req->ios2_Data = data;
  req->ios2_DataLength = data_len;

  BeginIO((struct IORequest *)req);
  return TRUE;
}

BOOL sanadev_io_read_start_orphan(sanadev_handle_t *sh, APTR data, ULONG data_len, BOOL raw)
{
  if(sh->read_pr.port == NULL) {
    return FALSE;
  }

  struct IOSana2Req *req = sh->read_pr.req;
  req->ios2_Req.io_Flags = raw ? SANA2IOF_RAW : 0;
  req->ios2_Req.io_Command = S2_READORPHAN;
  req->ios2_Data = data;
  req->ios2_DataLength = data_len;

  BeginIO((struct IORequest *)req);
  return TRUE;
}

BOOL sanadev_io_read_stop(sanadev_handle_t *sh)
{
  struct IORequest *req = (struct IORequest *)sh->read_pr.req;
  if(req == NULL) {
    return FALSE;
  }

  if (!CheckIO(req))
  {
    AbortIO(req);
  }
  WaitIO(req);
  return TRUE;
}

ULONG sanadev_io_read_get_mask(sanadev_handle_t *sh)
{
  return get_port_req_mask(&sh->read_pr);
}

BOOL sanadev_io_read_result(sanadev_handle_t *sh, UWORD *pkt_type, sanadev_mac_t dst_addr, UBYTE **data, ULONG *data_len)
{
  struct IOSana2Req *req = get_port_req_next_req(&sh->read_pr);
  if (req != NULL)
  {
    if(req->ios2_Req.io_Error != 0) {
      return FALSE;
    }

    *pkt_type = req->ios2_PacketType;
    CopyMem(dst_addr, req->ios2_DstAddr, SANADEV_MAC_SIZE);
    *data = req->ios2_Data;
    *data_len = req->ios2_DataLength;
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

BOOL sanadev_io_read_result_raw(sanadev_handle_t *sh, UBYTE **data, ULONG *data_len)
{
  struct IOSana2Req *req = get_port_req_next_req(&sh->read_pr);
  if (req != NULL)
  {
    if(req->ios2_Req.io_Error != 0) {
      return FALSE;
    }

    *data = req->ios2_Data;
    *data_len = req->ios2_DataLength;
    return TRUE;
  }
  else
  {
    return FALSE;
  }
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
    if(sana_req->ios2_Req.io_Error != 0) {
      return FALSE;
    }
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

static void get_error(struct IOSana2Req *sana_req, BYTE *error, ULONG *wire_error)
{
  *error = sana_req->ios2_Req.io_Error;
  *wire_error = sana_req->ios2_WireError;
}

static void print_error(struct IOSana2Req *sana_req)
{
  Printf((STRPTR) "SANA-II IO failed: cmd=%04lx -> error=%ld (%s) wire_error=%ld (%s)\n",
         (ULONG)sana_req->ios2_Req.io_Command,
         (LONG)sana_req->ios2_Req.io_Error,
         sanadev_error_string(sana_req->ios2_Req.io_Error),
         (ULONG)sana_req->ios2_WireError,
         sanadev_wire_error_string(sana_req->ios2_WireError));
}

void sanadev_cmd_get_error(sanadev_handle_t *sh, BYTE *error, ULONG *wire_error)
{
  get_error(sh->cmd_pr.req, error, wire_error);
}

void sanadev_cmd_print_error(sanadev_handle_t *sh)
{
  print_error(sh->cmd_pr.req);
}

void sanadev_io_write_get_error(sanadev_handle_t *sh, BYTE *error, ULONG *wire_error)
{
  get_error(sh->write_pr.req, error, wire_error);
}

void sanadev_io_write_print_error(sanadev_handle_t *sh)
{
  print_error(sh->write_pr.req);
}

void sanadev_io_read_get_error(sanadev_handle_t *sh, BYTE *error, ULONG *wire_error)
{
  get_error(sh->read_pr.req, error, wire_error);
}

void sanadev_io_read_print_error(sanadev_handle_t *sh)
{
  print_error(sh->read_pr.req);
}

void sanadev_print_mac(sanadev_mac_t mac)
{
  Printf("%02lx:%02lx:%02lx:%02lx:%02lx:%02lx",
         (ULONG)mac[0], (ULONG)mac[1], (ULONG)mac[2],
         (ULONG)mac[3], (ULONG)mac[4], (ULONG)mac[5]);
}

char *sanadev_error_string(BYTE error)
{
  switch(error) {
  case S2ERR_NO_ERROR: return "NO_ERROR";
  case S2ERR_NO_RESOURCES: return "NO_RESOURCES";
  case S2ERR_BAD_ARGUMENT: return "BAD_ARGUMENT";
  case S2ERR_BAD_STATE: return "BAD_STATE";
  case S2ERR_BAD_ADDRESS: return "BAD_ADDRRESSS";
  case S2ERR_MTU_EXCEEDED: return "MTU_EXCEEDED";
  case S2ERR_NOT_SUPPORTED: return "NOT_SUPPORTED";
  case S2ERR_SOFTWARE: return "SOFTWARE";
  case S2ERR_OUTOFSERVICE: return "OUTOFSERVICE";
  case S2ERR_TX_FAILURE: return "TX_FAILURE";
  default: return "?";
  }
}

char *sanadev_wire_error_string(ULONG wire_error)
{
  switch(wire_error) {
  case S2WERR_GENERIC_ERROR: return "GENERIC_ERROR";
  case S2WERR_NOT_CONFIGURED: return "NOT_CONFIGURED";
  case S2WERR_UNIT_ONLINE: return "ONLINE";
  case S2WERR_UNIT_OFFLINE: return "OFFLINE";
  case S2WERR_ALREADY_TRACKED: return "ALREADY_TRACKED";
  case S2WERR_NOT_TRACKED: return "NOT_TRACKED";
  case S2WERR_BUFF_ERROR: return "BUF_ERROR";
  case S2WERR_SRC_ADDRESS: return "SRC_ADDRESS";
  case S2WERR_DST_ADDRESS: return "DST_ADDRESS";
  case S2WERR_BAD_BROADCAST: return "BAD_BROADCAST";
  case S2WERR_BAD_MULTICAST: return "BAD_MULTICAST";
  case S2WERR_MULTICAST_FULL: return "MULTICAST_FULL";
  case S2WERR_BAD_EVENT: return "BAD_EVENT";
  case S2WERR_BAD_STATDATA: return "BAD_STATDATA";
  default: return "?";
  }
}
