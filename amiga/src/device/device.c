#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/cia.h>
#include <proto/misc.h>
#include <proto/utility.h>
#include <proto/timer.h>

#include <devices/sana2.h>
#include <devices/newstyle.h>
#include <hardware/cia.h>
#include <dos/dostags.h>
#include <resources/misc.h>
#include <exec/memory.h>

#include <string.h>

#include "global.h"
#include "debug.h"
#include "compiler.h"
#include "device.h"
#include "hw.h"

#include "devices/plipbox.h"
#include "devices/sana2link.h"

void SAVEDS ServerTask(void);
BOOL remtracktype(BASEPTR, ULONG type);
BOOL addtracktype(BASEPTR, ULONG type);
BOOL gettrackrec(BASEPTR, ULONG type, struct Sana2PacketTypeStats *info);
void dotracktype(BASEPTR, ULONG type, ULONG ps, ULONG pr, ULONG bs, ULONG br, ULONG pd);
void freetracktypes(BASEPTR);
static BOOL isinlist(struct Node *n, struct List *l);
static void abort_req(BASEPTR, struct IOSana2Req *ior);

static const UWORD supported_commands[] =
{
  CMD_READ,
  CMD_WRITE,
  S2_DEVICEQUERY,
  S2_GETSTATIONADDRESS,
  S2_CONFIGINTERFACE,
  //S2_ADDMULTICASTADDRESS,
  //S2_DELMULTICASTADDRESS,
  //S2_MULTICAST,
  S2_BROADCAST,
  S2_TRACKTYPE,
  S2_UNTRACKTYPE,
  S2_GETTYPESTATS,
  S2_GETSPECIALSTATS,
  S2_GETGLOBALSTATS,
  S2_ONEVENT,
  S2_READORPHAN,
  S2_ONLINE,
  S2_OFFLINE,
  NSCMD_DEVICEQUERY,
  //S2_ADDMULTICASTADDRESSES,
  //S2_DELMULTICASTADDRESSES,
  //S2_GETSIGNALQUALITY,
  //S2_GETNETWORKS,
  //S2_SETOPTIONS,
  //S2_SETKEY,
  //S2_GETNETWORKINFO,
  //S2_GETCRYPTTYPES,
  S2_LINK_STATUS,
  0
};

/*
** various support routines
*/
static BOOL isinlist(struct Node *n, struct List *l)
{
  struct Node *cmp;

  for (cmp = l->lh_Head; cmp->ln_Succ; cmp = cmp->ln_Succ)
    if (cmp == n)
      return TRUE;

  return FALSE;
}
static void abort_req(BASEPTR, struct IOSana2Req *ior)
{
  Remove((struct Node *)ior);
  ior->ios2_Req.io_Error = IOERR_ABORTED;
  ior->ios2_WireError = 0;
  ReplyMsg((struct Message *)ior);
}

/*
** initialise device
*/
ASM SAVEDS struct Device *DevInit(REG(d0, BASEPTR), REG(a0, BPTR seglist), REG(a6, struct Library *_SysBase))
{
  BOOL ok;
  UBYTE *p;
  UWORD i;

  d2(("dev_init\n"));

  device_init(&pb->pb_DevNode);

  /* clear data base */
  for (p = ((UBYTE *)pb) + sizeof(struct Library), i = sizeof(struct PLIPBase) - sizeof(struct Library); i; i--)
    *p++ = 0;

  SysBase = _SysBase;

  pb->pb_SegList = seglist; /* store DOS segment list */

  /* init some default values */
  pb->pb_Flags = PLIPF_OFFLINE;
  pb->pb_BPS = HW_BPS;
  pb->pb_MTU = HW_ETH_MTU;
  pb->pb_LinkStatus = S2LINKSTATUS_UNKNOWN;

  /* initialise the lists */
  NewList((struct List *)&pb->pb_ReadList);
  NewList((struct List *)&pb->pb_WriteList);
  NewList((struct List *)&pb->pb_EventList);
  NewList((struct List *)&pb->pb_ReadOrphanList);
  NewList((struct List *)&pb->pb_TrackList);
  NewList((struct List *)&pb->pb_BufferManagement);
  NewList((struct List *)&pb->pb_LinkStatusList);

  /* initialise the access protection semaphores */
  InitSemaphore(&pb->pb_ReadListSem);
  InitSemaphore(&pb->pb_ReadOrphanListSem);
  InitSemaphore(&pb->pb_EventListSem);
  InitSemaphore(&pb->pb_WriteListSem);
  InitSemaphore(&pb->pb_TrackListSem);
  InitSemaphore(&pb->pb_LinkStatusListSem);
  InitSemaphore(&pb->pb_Lock);

  pb->pb_SpecialStats[S2SS_TXERRORS].Type = S2SS_PLIP_TXERRORS;
  pb->pb_SpecialStats[S2SS_TXERRORS].Count = 0;
  pb->pb_SpecialStats[S2SS_TXERRORS].String = (STRPTR) "TX Errors";
  pb->pb_SpecialStats[S2SS_COLLISIONS].Type = S2SS_PLIP_COLLISIONS;
  pb->pb_SpecialStats[S2SS_COLLISIONS].Count = 0;
  pb->pb_SpecialStats[S2SS_COLLISIONS].String = (STRPTR) "Collisions";

  ok = FALSE;

  if (UtilityBase = OpenLibrary((STRPTR) "utility.library", 37))
  {
    if (DOSBase = OpenLibrary((STRPTR) "dos.library", 37))
    {
      ok = TRUE;
    }
    else
    {
      d(("ERROR: no dos lib\n"));
    }

    if (!ok)
      CloseLibrary(UtilityBase);
  }
  else
  {
    d(("ERROR: no utility\n"));
  }

  d2(("dev_init: res=%ld\n", ok));

  return (struct Device *)(ok ? pb : NULL);
}

/*
** open device
*/
ASM SAVEDS LONG DevOpen(REG(a1, struct IOSana2Req *ios2), REG(d0, ULONG unit), REG(d1, ULONG flags), REG(a6, BASEPTR))
{
  BOOL ok = FALSE;
  struct BufferManagement *bm;
  LONG rv;

  d2(("dev_open: entered\n"));

  /* Make sure our open remains single-threaded. */
  ObtainSemaphore(&pb->pb_Lock);

  pb->pb_DevNode.lib_OpenCnt++;

  /* not promiscouos mode and unit valid ? */
  if (!(flags & SANA2OPF_PROM) && (unit == 0))
  {
    /* Allow access only if NOT:
    **
    **    Anybody else has already opened it AND
    **          the current access is exclusive
    **       OR the first access was exclusive
    **       OR this access wants a different unit
    */

    if (!((pb->pb_DevNode.lib_OpenCnt > 1) &&
          ((flags & SANA2OPF_MINE) || (pb->pb_Flags & PLIPF_EXCLUSIVE) || (unit != pb->pb_Unit))))
    {
      if (flags & SANA2OPF_MINE)
        pb->pb_Flags |= PLIPF_EXCLUSIVE;
      else
        pb->pb_Flags &= ~PLIPF_EXCLUSIVE;

      /*
      ** 13.05.96: Detlef Wuerkner <TetiSoft@apg.lahn.de>
      ** Rememer unit of 1st OpenDevice()
      */
      if (pb->pb_DevNode.lib_OpenCnt == 1) {
        pb->pb_Unit = unit;
      }

#ifdef ENABLE_TIMING
      /* enable req timing? */
      if (flags & S2PB_OPF_REQ_TIMING) {
        d2(("enable req timing!"));
        pb->pb_Flags |= PLIPF_REQ_TIMING;
      }
#endif

      /*
      ** Each opnener get's it's own BufferManagement. This is neccessary
      ** since we want to allow several protocol stacks to use PLIP
      ** simultaneously.
      */
      if ((bm = (struct BufferManagement *)AllocVec(sizeof(struct BufferManagement), MEMF_CLEAR | MEMF_ANY)) != NULL)
      {
        /*
        ** We don't care if there actually are buffer management functions,
        ** because there might be openers who just want some statistics
        ** from us.
        */
        bm->bm_CopyToBuffer = (BMFunc)GetTagData(S2_CopyToBuff, 0,
                                                 (struct TagItem *)ios2->ios2_BufferManagement);
        bm->bm_CopyFromBuffer = (BMFunc)GetTagData(S2_CopyFromBuff, 0,
                                                   (struct TagItem *)ios2->ios2_BufferManagement);

        d2(("task: setup\n"));
        if (!pb->pb_Server)
        {
          volatile struct ServerStartup ss;
          struct MsgPort *port;

          if (port = CreateMsgPort())
          {
            d2(("task: start"));
            if (pb->pb_Server = CreateNewProcTags(NP_Entry, (ULONG)ServerTask, NP_Name,
                                                  (ULONG)SERVERTASKNAME, TAG_DONE))
            {
              ss.ss_Error = 1;
              ss.ss_PLIPBase = pb;
              ss.ss_Msg.mn_Length = sizeof(ss);
              ss.ss_Msg.mn_ReplyPort = port;
              d2(("task: passing startup msg, pb is %lx\n", pb));
              PutMsg(&pb->pb_Server->pr_MsgPort, (struct Message *)&ss);
              WaitPort(port);

              if (!ss.ss_Error)
                ok = TRUE;
              else
              {
                d(("ERROR: server task failed\n"));
              }
            }
            else
            {
              d(("ERROR: couldn't launch server task\n"));
            }
            DeleteMsgPort(port);
          }
          else
          {
            d(("ERROR: no temp-message port for server startup\n"));
          }
        }
        else
          ok = TRUE;

        if (!ok)
          FreeVec(bm);
        else
        {
          /* enqueue buffer management into list
           */
          AddTail((struct List *)&pb->pb_BufferManagement, (struct Node *)bm);
          pb->pb_DevNode.lib_OpenCnt++;
          pb->pb_DevNode.lib_Flags &= ~LIBF_DELEXP;
          ios2->ios2_BufferManagement = (void *)bm;
          ios2->ios2_Req.io_Error = 0;
          ios2->ios2_Req.io_Unit = (struct Unit *)unit;
          ios2->ios2_Req.io_Device = (struct Device *)pb;
          rv = 0;
        }
      }
    }
  }

  /* See if something went wrong. */
  if (!ok)
  {
    ios2->ios2_Req.io_Error = IOERR_OPENFAIL;
    ios2->ios2_Req.io_Unit = (struct Unit *)0;
    ios2->ios2_Req.io_Device = (struct Device *)0;
    rv = IOERR_OPENFAIL;
  }
  ios2->ios2_Req.io_Message.mn_Node.ln_Type = NT_REPLYMSG;

  pb->pb_DevNode.lib_OpenCnt--;
  ReleaseSemaphore(&pb->pb_Lock);

  return rv;
}

/*
** close device
*/
ASM SAVEDS BPTR DevClose(REG(a1, struct IOSana2Req *ior), REG(a6, BASEPTR))
{
  BPTR seglist;
  struct BufferManagement *bm;

  d2(("dev_close\n"));

  ObtainSemaphore(&pb->pb_Lock);

  /* invalidate IO request block */
  ior->ios2_Req.io_Device = (struct Device *)-1;
  ior->ios2_Req.io_Unit = (struct Unit *)-1;

  /* search and free BuffMgmt structure */
  for (bm = (struct BufferManagement *)pb->pb_BufferManagement.lh_Head;
       bm->bm_Node.mln_Succ; bm = (struct BufferManagement *)bm->bm_Node.mln_Succ)
    if (bm == ior->ios2_BufferManagement)
    {
      Remove((struct Node *)bm);
      FreeVec(bm);
      break;
    }

  pb->pb_DevNode.lib_OpenCnt--;

  ReleaseSemaphore(&pb->pb_Lock);

  if (pb->pb_DevNode.lib_Flags & LIBF_DELEXP)
    seglist = DevExpunge(pb);
  else
    seglist = 0;

  d2(("dev_close: done\n"));

  return seglist;
}

ASM SAVEDS BPTR DevExpunge(REG(a6, BASEPTR))
{
  BPTR seglist;
  ULONG sigb;

  d2(("dev_expunge\n"));

  if (pb->pb_DevNode.lib_OpenCnt)
  {
    pb->pb_DevNode.lib_Flags |= LIBF_DELEXP;
    seglist = 0;
  }
  else
  {
    /* detach device from system list */
    Remove((struct Node *)pb);

    /* stop the servr task */
    d2(("killing server task\n"));
    pb->pb_Task = FindTask(0L);
    /* We must allocate a new signal, as we don't know in whose
    ** context we're running. If we get no signal, we poll
    ** for the server-exits flag.
    */
    sigb = AllocSignal(-1);
    pb->pb_ServerStoppedSigMask = (sigb == -1) ? 0 : (1 << sigb);
    Signal((struct Task *)pb->pb_Server, SIGBREAKF_CTRL_C);
    if (pb->pb_ServerStoppedSigMask)
    {
      Wait(pb->pb_ServerStoppedSigMask);
      FreeSignal(sigb);
    }
    else
    {
      while (!(pb->pb_Flags & PLIPF_SERVERSTOPPED))
        Delay(10);
    }
    d2(("server task has gone\n"));

    /* clean up track */
    freetracktypes(pb);

    CloseLibrary(UtilityBase);
    CloseLibrary(DOSBase);

    /* save seglist for return value */
    seglist = (long)pb->pb_SegList;

    /* return memory
    **
    ** NO FURTHER ACCESS TO PLIPBase ALLOWED!
    */
    FreeMem(((char *)pb) - pb->pb_DevNode.lib_NegSize,
            (ULONG)(pb->pb_DevNode.lib_PosSize + pb->pb_DevNode.lib_NegSize));
  }

  d2(("dev_expunge: done\n"));

  return seglist;
}

/*
** initiate io command (1st level dispatcher)
*/
static INLINE void DevForwardIO(BASEPTR, struct IOSana2Req *ios2)
{
  d2(("forwarding request %ld\n", ios2->ios2_Req.io_Command));

  /* request is no longer of type "quick i/o" */
  ios2->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
  PutMsg(pb->pb_ServerPort, (struct Message *)ios2);
}

void DevTermIO(BASEPTR, struct IOSana2Req *ios2)
{
  d2(("dev_termio: cmd=%ld, error=%ld, wireerror=%ld\n", (ULONG)ios2->ios2_Req.io_Command, ios2->ios2_Req.io_Error, ios2->ios2_WireError));

#ifdef ENABLE_TIMING
  /* req timing? */
  if((pb->pb_Flags & PLIPF_REQ_TIMING) && (ios2->ios2_StatData != NULL)) {
    s2pb_req_timing_t *rt = (s2pb_req_timing_t *)ios2->ios2_StatData;
    hw_get_eclock(pb, &rt->end_req);
  }
#endif

  /* if this command was done asynchonously, we must
  ** reply the request
  */
  if (!(ios2->ios2_Req.io_Flags & SANA2IOF_QUICK))
    ReplyMsg((struct Message *)ios2);
  else /* otherwise just mark it as done */
    ios2->ios2_Req.io_Message.mn_Node.ln_Type = NT_REPLYMSG;
}

static REGARGS BOOL handle_link_status(BASEPTR, struct IOSana2Req *ios2,
                                       struct Sana2LinkStatus *s2_link_status)
{
  /* Sanity Check of stat data */
  if(s2_link_status == NULL) {
    ios2->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
    ios2->ios2_WireError = S2WERR_BAD_STATDATA;
    return FALSE;
  }
  if(s2_link_status->s2ls_Size < sizeof(struct Sana2LinkStatus)) {
    ios2->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
    ios2->ios2_WireError = S2WERR_BAD_STATDATA;
    return FALSE;
  }

  /* copy current state to old state */
  s2_link_status->s2ls_PreviousStatus = s2_link_status->s2ls_CurrentStatus;

  /* retrieve link status */
  BYTE link_status = pb->pb_LinkStatus;

  /* if mode is IMMEDIATE or current status is different from real state
     then return status directly */
  if((s2_link_status->s2ls_QueryMode == S2LS_QUERYMODE_IMMEDIATE) ||
     (s2_link_status->s2ls_CurrentStatus != link_status)) {

    /* get time stamp */
    hw_get_eclock(pb, (S2QUAD *)&s2_link_status->s2ls_TimeStamp);

    s2_link_status->s2ls_CurrentStatus = link_status;

    /* do not queue request */
    return FALSE;
  }
  /* otherwise add to watch list and update req on next change */
  else {
    return TRUE;
  }
}

static REGARGS void handle_newstyle_query(BASEPTR, struct IOStdReq *request)
{
  d4r(("NSCMD_DEVICEQUERY\n"));

  /* expected min size */
  ULONG size = sizeof(struct NSDeviceQueryResult);
  /* sanity checks */
  if((request->io_Data == NULL) || (request->io_Length < size)) {
    request->io_Error = IOERR_BADLENGTH;
  }
  else {
    struct NSDeviceQueryResult *info = request->io_Data;
    request->io_Error = 0;
    request->io_Actual = size;
    info->nsdqr_SizeAvailable = size;
    info->nsdqr_DevQueryFormat = 0;
    info->nsdqr_DeviceType = NSDEVTYPE_SANA2;
    info->nsdqr_DeviceSubType = 0;
    info->nsdqr_SupportedCommands = (APTR)supported_commands;
  }

  if (!(request->io_Flags & IOF_QUICK)) {
    request->io_Message.mn_Node.ln_Type = NT_MESSAGE;
    ReplyMsg((struct Message *)request);
  }
  else {
    /* otherwise just mark it as done */
    request->io_Message.mn_Node.ln_Type = NT_REPLYMSG;
  }
}

ASM SAVEDS void DevBeginIO(REG(a1, struct IOSana2Req *ios2), REG(a6, BASEPTR))
{
  /* handle newstyle query */
  struct IOStdReq *ioreq = (struct IOStdReq *)ios2;
  if(ioreq->io_Command == NSCMD_DEVICEQUERY) {
    handle_newstyle_query(pb, ioreq);
    return;
  }

  ULONG mtu;
  ULONG oldWire = ios2->ios2_WireError;

  /* mark request as active */
  ios2->ios2_Req.io_Message.mn_Node.ln_Type = NT_MESSAGE;
  ios2->ios2_Req.io_Error = S2ERR_NO_ERROR;
  ios2->ios2_WireError = S2WERR_GENERIC_ERROR;

  d2(("dev_beginio: cmd=%ld req=%lx\n", (ULONG)ios2->ios2_Req.io_Command, (ULONG)ios2));

#ifdef ENABLE_TIMING
  /* req timing? */
  if((pb->pb_Flags & PLIPF_REQ_TIMING) && (ios2->ios2_StatData != NULL)) {
    s2pb_req_timing_t *rt = (s2pb_req_timing_t *)ios2->ios2_StatData;
    hw_get_eclock(pb, &rt->start_req);
  }
#endif

  /*
  ** 1st level command dispatcher
  **
  ** Here we decide wether we can process the request immediately avoiding
  ** a task switch. This is called "Quick I/O" and signalled to DoIO()
  ** by setting the node type of the request to NT_REPLYMSG (see TermIO()).
  **
  ** Otherwise, we clear the SANA2IOF_QUICK flag and forward the request to
  ** the server. We may NEVER again access the request structure after
  ** the PutMsg()! The server - running at a high priority - will peempt
  ** us and might complety satisfy the request before we will be wakened up
  ** again.
  ** The same goes for those requests that we put into the server's queue.
  */
  switch (ios2->ios2_Req.io_Command)
  {
  case CMD_READ:
    d4r(("CMD_READ %ld\n", ios2->ios2_DataLength));
    if (pb->pb_Flags & PLIPF_OFFLINE)
    {
      ios2->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
      ios2->ios2_WireError = S2WERR_UNIT_OFFLINE;
    }
    else if (ios2->ios2_BufferManagement == NULL)
    {
      ios2->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
      ios2->ios2_WireError = S2WERR_BUFF_ERROR;
    }
    else
    {
      ios2->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
      ObtainSemaphore(&pb->pb_ReadListSem);
      AddTail((struct List *)&pb->pb_ReadList, (struct Node *)ios2);
      ReleaseSemaphore(&pb->pb_ReadListSem);
      ios2 = NULL;
    }
    break;

  case S2_BROADCAST:
    d4r(("S2_BROADCAST\n"));
    /* set broadcast addr: ff:ff:ff:ff:ff:ff */
    memset(ios2->ios2_DstAddr, 0xff, HW_ADDRFIELDSIZE);
    /* fall through */
  case CMD_WRITE:
    d4r(("CMD_WRITE %ld\n", ios2->ios2_DataLength));
    /* determine max valid size */
    mtu = pb->pb_MTU;
    if (ios2->ios2_Req.io_Flags & SANA2IOF_RAW)
    {
      mtu += HW_ETH_HDR_SIZE;
    }
    if (ios2->ios2_DataLength > mtu)
    {
      ios2->ios2_Req.io_Error = S2ERR_MTU_EXCEEDED;
    }
    else if (ios2->ios2_BufferManagement == NULL)
    {
      ios2->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
      ios2->ios2_WireError = S2WERR_BUFF_ERROR;
    }
    else if (pb->pb_Flags & PLIPF_OFFLINE)
    {
      ios2->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
      ios2->ios2_WireError = S2WERR_UNIT_OFFLINE;
    }
    else
    {
      ios2->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
      ios2->ios2_Req.io_Error = 0;
      ObtainSemaphore(&pb->pb_WriteListSem);
      AddTail((struct List *)&pb->pb_WriteList, (struct Node *)ios2);
      ReleaseSemaphore(&pb->pb_WriteListSem);
      Signal((struct Task *)pb->pb_Server, SIGBREAKF_CTRL_F);
      ios2 = NULL;
    }
    break;

  case S2_ONLINE:
  case S2_OFFLINE:
  case S2_CONFIGINTERFACE: /* forward request */
    DevForwardIO(pb, ios2);
    ios2 = NULL;
    break;

  case S2_GETSTATIONADDRESS:
    d4r(("S2_GETSTATIONADDRESS\n"));
    memcpy(ios2->ios2_SrcAddr, pb->pb_CfgAddr, HW_ADDRFIELDSIZE);
    memcpy(ios2->ios2_DstAddr, pb->pb_DefAddr, HW_ADDRFIELDSIZE);
    break;

  case S2_DEVICEQUERY:
  {
    d4r(("S2_DEVICEQUERY\n"));
    struct Sana2DeviceQuery *devquery;

    devquery = ios2->ios2_StatData;
    devquery->DevQueryFormat = 0; /* "this is format 0" */
    devquery->DeviceLevel = 0;    /* "this spec defines level 0" */

    if (devquery->SizeAvailable >= 18)
      devquery->AddrFieldSize = HW_ADDRFIELDSIZE * 8; /* Bits! */
    if (devquery->SizeAvailable >= 22)
      devquery->MTU = pb->pb_MTU;
    if (devquery->SizeAvailable >= 26)
      devquery->BPS = pb->pb_BPS;
    if (devquery->SizeAvailable >= 30)
      devquery->HardwareType = S2WireType_Ethernet;

    devquery->SizeSupplied = min((int)devquery->SizeAvailable, 30);
  }
  break;

  case S2_ONEVENT:
    d4r(("S2_ONEVENT: %lx\n", oldWire));
    /* Two special cases. S2EVENT_ONLINE and S2EVENT_OFFLINE are supposed to
       retun immediately if we are already in the state that they are waiting
       for. */
    if (((oldWire & S2EVENT_ONLINE) && !(pb->pb_Flags & PLIPF_OFFLINE)) ||
        ((oldWire & S2EVENT_OFFLINE) && (pb->pb_Flags & PLIPF_OFFLINE)))
    {
      ios2->ios2_Req.io_Error = 0;
      ios2->ios2_WireError = oldWire & (S2EVENT_ONLINE | S2EVENT_OFFLINE);
      DevTermIO(pb, ios2);
      ios2 = NULL;
    }
    else if ((oldWire & (S2EVENT_ERROR | S2EVENT_TX | S2EVENT_RX | S2EVENT_ONLINE |
                         S2EVENT_OFFLINE | S2EVENT_BUFF | S2EVENT_HARDWARE | S2EVENT_SOFTWARE)) != oldWire)
    {
      /* we cannot handle such events */
      ios2->ios2_Req.io_Error = S2ERR_NOT_SUPPORTED;
      ios2->ios2_WireError = S2WERR_BAD_EVENT;
    }
    else
    {
      /* Queue anything else */
      ios2->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
      ios2->ios2_WireError = oldWire;
      ObtainSemaphore(&pb->pb_EventListSem);
      AddTail((struct List *)&pb->pb_EventList, (struct Node *)ios2);
      ReleaseSemaphore(&pb->pb_EventListSem);
      ios2 = NULL;
    }
    break;

    /* ----- new link status command ----- */

  case S2_LINK_STATUS:
    {
      d4r(("S2_LINK_STATUS\n"));
      struct Sana2LinkStatus *s2_link_status = ios2->ios2_StatData;
      BOOL queue = handle_link_status(pb, ios2, s2_link_status);
      if(queue) {
        ios2->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
        ObtainSemaphore(&pb->pb_LinkStatusListSem);
        AddTail((struct List *)&pb->pb_LinkStatusList, (struct Node *)ios2);
        ReleaseSemaphore(&pb->pb_LinkStatusListSem);
        ios2 = NULL;
      }
    }
    break;

    /* --------------- stats support ----------------------- */

  case S2_TRACKTYPE:
    d4r(("S2_TRACKTYPE\n"));
    if (!addtracktype(pb, ios2->ios2_PacketType))
    {
      ios2->ios2_Req.io_Error = S2ERR_NO_RESOURCES;
    }
    break;

  case S2_UNTRACKTYPE:
    d4r(("S2_UNTRACKTYPE\n"));
    if (!remtracktype(pb, ios2->ios2_PacketType))
    {
      ios2->ios2_Req.io_Error = S2ERR_BAD_STATE;
      ios2->ios2_WireError = S2WERR_NOT_TRACKED;
    }
    break;

  case S2_GETTYPESTATS:
    d4r(("S2_GETTYPESTATS\n"));
    if (!gettrackrec(pb, ios2->ios2_PacketType, (struct Sana2PacketTypeStats *)ios2->ios2_StatData))
    {
      ios2->ios2_Req.io_Error = S2ERR_BAD_STATE;
      ios2->ios2_WireError = S2WERR_NOT_TRACKED;
    }
    break;

  case S2_READORPHAN:
    d4r(("S2_READORPHAN\n"));
    if (pb->pb_Flags & PLIPF_OFFLINE)
    {
      ios2->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
      ios2->ios2_WireError = S2WERR_UNIT_OFFLINE;
    }
    else if (ios2->ios2_BufferManagement == NULL)
    {
      ios2->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
      ios2->ios2_WireError = S2WERR_BUFF_ERROR;
    }
    else
    { /* Enqueue it to the orphan-reader-list */
      ios2->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
      ObtainSemaphore(&pb->pb_ReadOrphanListSem);
      AddTail((struct List *)&pb->pb_ReadOrphanList, (struct Node *)ios2);
      ReleaseSemaphore(&pb->pb_ReadOrphanListSem);
      ios2 = NULL;
    }
    break;

  case S2_GETGLOBALSTATS:
    d4r(("S2_GETGLOBALSTATS\n"));
    memcpy(ios2->ios2_StatData, &pb->pb_DevStats, sizeof(struct Sana2DeviceStats));
    break;

  case S2_GETSPECIALSTATS:
  {
    d4r(("S2_GETSPECIALSTATS\n"));
    struct Sana2SpecialStatHeader *s2ssh = (struct Sana2SpecialStatHeader *)ios2->ios2_StatData;

    if (pb->pb_ExtFlags & PLIPEF_NOSPECIALSTATS)
    {
      s2ssh->RecordCountSupplied = 0;
    }
    else
    {
      s2ssh->RecordCountSupplied = s2ssh->RecordCountMax > S2SS_COUNT ? S2SS_COUNT : s2ssh->RecordCountMax;
      CopyMem(pb->pb_SpecialStats, (void *)(s2ssh + 1),
              s2ssh->RecordCountSupplied * sizeof(struct Sana2SpecialStatRecord));
    }
  }
  break;

    /* --------------- unsupported requests -------------------- */

    /* all standard commands we don't support */
  case CMD_RESET:
  case CMD_UPDATE:
  case CMD_CLEAR:
  case CMD_STOP:
  case CMD_START:
  case CMD_FLUSH:
    d4r(("CMD_%lx -> NOCMD\n", (ULONG)ios2->ios2_Req.io_Command));
    ios2->ios2_Req.io_Error = IOERR_NOCMD;
    ios2->ios2_WireError = S2WERR_GENERIC_ERROR;
    break;

    /* other commands (SANA-2) we don't support */
  /*case S2_ADDMULTICASTADDRESS:*/
  /*case S2_DELMULTICASTADDRESS:*/
  /*case S2_MULTICAST:*/
  default:
  {
    /* ask hw driver if its a special command */
    BOOL is_cmd = hw_can_handle_special_cmd(pb, ios2->ios2_Req.io_Command);
    if (is_cmd)
    {
      ios2->ios2_WireError = oldWire;
      DevForwardIO(pb, ios2);
      ios2 = NULL;
    }
    else
    {
      d4r(("CMD_%lx -> NOT_SUPPORTED\n", (ULONG)ios2->ios2_Req.io_Command));
      ios2->ios2_Req.io_Error = S2ERR_NOT_SUPPORTED;
      ios2->ios2_WireError = S2WERR_GENERIC_ERROR;
    }
  }
  break;
  }

  if (ios2)
    DevTermIO(pb, ios2);

  d2(("dev_beginio: cmd=%ld req=%lx done\n", (ULONG)ios2->ios2_Req.io_Command, (ULONG)ios2));

  return;
}

/*
** stop io-command
*/
ASM SAVEDS LONG DevAbortIO(REG(a1, struct IOSana2Req *ior), REG(a6, BASEPTR))
{
  BOOL is;
  LONG rc = 0;

  d2(("dev_abortio: cmd=%ld\n", (ULONG)ior->ios2_Req.io_Command));

  ObtainSemaphore(&pb->pb_WriteListSem);
  if ((is = isinlist((struct Node *)ior, (struct List *)&pb->pb_WriteList)))
    abort_req(pb, ior);
  ReleaseSemaphore(&pb->pb_WriteListSem);
  if (is)
    goto leave;

  ObtainSemaphore(&pb->pb_ReadListSem);
  if ((is = isinlist((struct Node *)ior, (struct List *)&pb->pb_ReadList)))
    abort_req(pb, ior);
  ReleaseSemaphore(&pb->pb_ReadListSem);
  if (is)
    goto leave;

  ObtainSemaphore(&pb->pb_EventListSem);
  if ((is = isinlist((struct Node *)ior, (struct List *)&pb->pb_EventList)))
    abort_req(pb, ior);
  ReleaseSemaphore(&pb->pb_EventListSem);
  if (is)
    goto leave;

  ObtainSemaphore(&pb->pb_ReadOrphanListSem);
  if ((is = isinlist((struct Node *)ior, (struct List *)&pb->pb_ReadOrphanList)))
    abort_req(pb, ior);
  ReleaseSemaphore(&pb->pb_ReadOrphanListSem);
  if (is)
    goto leave;

  ObtainSemaphore(&pb->pb_LinkStatusListSem);
  if ((is = isinlist((struct Node *)ior, (struct List *)&pb->pb_LinkStatusList)))
    abort_req(pb, ior);
  ReleaseSemaphore(&pb->pb_LinkStatusListSem);
  if (is)
    goto leave;

  rc = -1;

leave:
  d2(("dev_abortio: cmd=%ld res=%ld\n", (ULONG)ior->ios2_Req.io_Command, rc));
  return rc;
}

#ifdef __GNUC__
#ifdef BASEREL
extern void __restore_a4(void)
{
  __asm volatile("\tlea ___a4_init, a4");
}
#endif
#endif
