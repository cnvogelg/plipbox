#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/cia.h>
#include <proto/misc.h>
#include <proto/utility.h>
#include <proto/timer.h>

#include <devices/sana2.h>
#include <hardware/cia.h>
#include <dos/dostags.h>
#include <resources/misc.h>
#include <exec/memory.h>

#include <string.h>

#include "global.h"
#include "debug.h"
#include "compiler.h"
#include "device.h"

PUBLIC VOID SAVEDS ServerTask(VOID);
PUBLIC BOOL remtracktype(BASEPTR, ULONG type);
PUBLIC BOOL addtracktype(BASEPTR, ULONG type);
PUBLIC BOOL gettrackrec(BASEPTR, ULONG type, struct Sana2PacketTypeStats *info);
PUBLIC VOID dotracktype(BASEPTR, ULONG type, ULONG ps, ULONG pr, ULONG bs, ULONG br, ULONG pd);
PUBLIC VOID freetracktypes(BASEPTR);
PRIVATE BOOL isinlist(struct Node *n, struct List *l);
PRIVATE VOID abort_req(BASEPTR, struct IOSana2Req *ior);

   /*
   ** various support routines
   */
PRIVATE BOOL isinlist(struct Node *n, struct List *l)
{
   struct Node *cmp;

   for(cmp = l->lh_Head; cmp->ln_Succ; cmp = cmp->ln_Succ)
      if (cmp == n) return TRUE;
   
   return FALSE;
}
PRIVATE VOID abort_req(BASEPTR, struct IOSana2Req *ior)
{
   Remove((struct Node*)ior);
   ior->ios2_Req.io_Error = IOERR_ABORTED;
   ior->ios2_WireError = 0;
   ReplyMsg((struct Message*)ior);
}

   /*
   ** initialise device
   */
PUBLIC ASM SAVEDS struct Device *DevInit(REG(d0,BASEPTR), REG(a0,BPTR seglist), REG(a6,struct Library *_SysBase))
{
   BOOL ok;
   UBYTE *p;
   UWORD i;

   d(("entered device, initialising PLIPBase...\n"));

   device_init(&pb->pb_DevNode);

      /* clear data base */
   for(p = ((UBYTE*)pb) + sizeof(struct Library), i = sizeof(struct PLIPBase)-sizeof(struct Library); i; i--)
      *p++ = 0;

   SysBase = _SysBase;

   pb->pb_SegList = seglist;           /* store DOS segment list */

      /* init some default values */
   pb->pb_Flags = PLIPF_OFFLINE;
   pb->pb_BPS = HW_BPS;
   pb->pb_MTU = HW_ETH_MTU;

      /* initialise the lists */
   NewList((struct List*)&pb->pb_ReadList);
   NewList((struct List*)&pb->pb_WriteList);
   NewList((struct List*)&pb->pb_EventList);
   NewList((struct List*)&pb->pb_ReadOrphanList);
   NewList((struct List*)&pb->pb_TrackList);
   NewList((struct List*)&pb->pb_BufferManagement);

      /* initialise the access protection semaphores */
   InitSemaphore(&pb->pb_ReadListSem);
   InitSemaphore(&pb->pb_ReadOrphanListSem);
   InitSemaphore(&pb->pb_EventListSem);
   InitSemaphore(&pb->pb_WriteListSem);
   InitSemaphore(&pb->pb_TrackListSem);
   InitSemaphore(&pb->pb_Lock);

   pb->pb_SpecialStats[S2SS_TXERRORS].Type = S2SS_PLIP_TXERRORS;
   pb->pb_SpecialStats[S2SS_TXERRORS].Count = 0;
   pb->pb_SpecialStats[S2SS_TXERRORS].String = (STRPTR)"TX Errors";
   pb->pb_SpecialStats[S2SS_COLLISIONS].Type = S2SS_PLIP_COLLISIONS;
   pb->pb_SpecialStats[S2SS_COLLISIONS].Count = 0;
   pb->pb_SpecialStats[S2SS_COLLISIONS].String = (STRPTR)"Collisions";

   ok = FALSE;

   if (UtilityBase = OpenLibrary((STRPTR)"utility.library", 37))
   {
      if (DOSBase = OpenLibrary((STRPTR)"dos.library", 37))
      {
         ok = TRUE;
      }
      else
      {
         d(("no dos\n"));
      }

      if (!ok) CloseLibrary(UtilityBase);
   }
   else
   {
      d(("no utility\n"));
   }

   d(("left %ld\n",ok));

   return (struct Device *)(ok ? pb : NULL);
}

   /*
   ** open device
   */
PUBLIC ASM SAVEDS LONG DevOpen(REG(a1,struct IOSana2Req *ios2), REG(d0,ULONG unit), REG(d1,ULONG flags), REG(a6,BASEPTR))
{
   BOOL ok = FALSE;
   struct BufferManagement *bm;
   LONG rv;

   d(("entered\n"));

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
            ((flags & SANA2OPF_MINE)
          || (pb->pb_Flags & PLIPF_EXCLUSIVE)
          || (unit != pb->pb_Unit))))
      {
         if (flags & SANA2OPF_MINE)
            pb->pb_Flags |= PLIPF_EXCLUSIVE;
         else
            pb->pb_Flags &= ~PLIPF_EXCLUSIVE;
         
         /*
         ** 13.05.96: Detlef Wuerkner <TetiSoft@apg.lahn.de>
         ** Rememer unit of 1st OpenDevice()
         */
         if (pb->pb_DevNode.lib_OpenCnt == 1)
            pb->pb_Unit = unit;

         /* setup default mac */
         {
            unsigned char addr[6] = { 0x1a,0x11,0xaf,0xa0,0x47,0x11};
            memcpy(pb->pb_CfgAddr, addr, HW_ADDRFIELDSIZE);
            memcpy(pb->pb_DefAddr, addr, HW_ADDRFIELDSIZE);
         }

         /*
         ** Each opnener get's it's own BufferManagement. This is neccessary
         ** since we want to allow several protocol stacks to use PLIP
         ** simultaneously.
         */
         if ((bm = (struct BufferManagement *)AllocVec(sizeof(struct BufferManagement),MEMF_CLEAR|MEMF_PUBLIC)) != NULL)
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

            d(("starting servertask\n"));
            if (!pb->pb_Server)
            {
               volatile struct ServerStartup ss;
               struct MsgPort *port;

               if (port = CreateMsgPort())
               {
                  d(("starting server"));
                  if (pb->pb_Server = CreateNewProcTags(NP_Entry, (ULONG)ServerTask, NP_Name,
                                                                  (ULONG)SERVERTASKNAME, TAG_DONE))
                  {
                     ss.ss_Error = 1;
                     ss.ss_PLIPBase = pb;
                     ss.ss_Msg.mn_Length = sizeof(ss);
                     ss.ss_Msg.mn_ReplyPort = port;
                     d(("passing startup msg, pb is %lx\n", pb));
                     PutMsg(&pb->pb_Server->pr_MsgPort, (struct Message*)&ss);
                     WaitPort(port);

                     if (!ss.ss_Error)
                        ok = TRUE;
                     else
                     {
                        d(("server task failed\n"));
                     }
                  }
                  else
                  {
                     d(("couldn't launch server task\n"));
                  }
                  DeleteMsgPort(port);
               }
               else
               {
                  d(("no temp-message port for server startup\n"));
               }
            }
            else ok = TRUE;

            if (!ok)
               FreeVec(bm);
            else
            {
               /* enqueue buffer management into list
               */
               AddTail((struct List *)&pb->pb_BufferManagement,(struct Node *)bm);
               pb->pb_DevNode.lib_OpenCnt++;
               pb->pb_DevNode.lib_Flags &= ~LIBF_DELEXP;
               ios2->ios2_BufferManagement = (VOID *)bm;
               ios2->ios2_Req.io_Error = 0;
               ios2->ios2_Req.io_Unit = (struct Unit *)unit;
               ios2->ios2_Req.io_Device = (struct Device *)pb;
               rv = 0;
            }
         }
      }
   }

      /* See if something went wrong. */
   if(!ok)
   {
      ios2->ios2_Req.io_Error = IOERR_OPENFAIL;
      ios2->ios2_Req.io_Unit = (struct Unit *) 0;
      ios2->ios2_Req.io_Device = (struct Device *) 0;
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
PUBLIC ASM SAVEDS BPTR DevClose(REG(a1,struct IOSana2Req *ior), REG(a6,BASEPTR))
{
   BPTR seglist;
   struct BufferManagement *bm;

   d2(("entered\n"));

   ObtainSemaphore(&pb->pb_Lock);

      /* invalidate IO request block */
   ior->ios2_Req.io_Device = (struct Device *)-1;
   ior->ios2_Req.io_Unit = (struct Unit *)-1;

      /* search and free BuffMgmt structure */
   for(bm = (struct BufferManagement *)pb->pb_BufferManagement.lh_Head;
       bm->bm_Node.mln_Succ; bm = (struct BufferManagement *)bm->bm_Node.mln_Succ)
      if (bm == ior->ios2_BufferManagement)
      {
         Remove((struct Node*)bm);
         FreeVec(bm);
         break;
      }

   pb->pb_DevNode.lib_OpenCnt--;

   ReleaseSemaphore(&pb->pb_Lock);

   if (pb->pb_DevNode.lib_Flags & LIBF_DELEXP)
      seglist = DevExpunge(pb);
   else
      seglist = 0;

   return seglist;
}


PUBLIC ASM SAVEDS BPTR DevExpunge(REG(a6,BASEPTR))
{
   BPTR seglist;
   ULONG sigb;

   d2(("entered\n"));

   if (pb->pb_DevNode.lib_OpenCnt)
   {
      pb->pb_DevNode.lib_Flags |= LIBF_DELEXP;
      seglist = 0;
   }
   else
   {
         /* detach device from system list */
      Remove((struct Node*)pb);

         /* stop the servr task */
      d2(("killing server task\n"));
      pb->pb_Task = FindTask(0L);
         /* We must allocate a new signal, as we don't know in whose
         ** context we're running. If we get no signal, we poll
         ** for the server-exits flag.
         */
      sigb = AllocSignal(-1);
      pb->pb_ServerStoppedSigMask = (sigb == -1) ? 0 : (1<<sigb);
      Signal((struct Task*)pb->pb_Server, SIGBREAKF_CTRL_C);
      if (pb->pb_ServerStoppedSigMask)
      {
         Wait(pb->pb_ServerStoppedSigMask);
         FreeSignal(sigb);
      }
      else
      {
         while(!(pb->pb_Flags & PLIPF_SERVERSTOPPED))
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
      FreeMem( ((char *) pb) - pb->pb_DevNode.lib_NegSize,
         (ULONG)(pb->pb_DevNode.lib_PosSize + pb->pb_DevNode.lib_NegSize));
   }

   return seglist;
}

   /*
   ** initiate io command (1st level dispatcher)
   */
static INLINE VOID DevForwardIO(BASEPTR, struct IOSana2Req *ios2)
{
   d(("forwarding request %ld\n", ios2->ios2_Req.io_Command));

   /* request is no longer of type "quick i/o" */
   ios2->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
   PutMsg(pb->pb_ServerPort, (struct Message*)ios2);
}
PUBLIC VOID DevTermIO(BASEPTR, struct IOSana2Req *ios2)
{
   d(("cmd = %ld, error = %ld, wireerror = %ld\n", ios2->ios2_Req.io_Command, ios2->ios2_Req.io_Error,ios2->ios2_WireError));

                  /* if this command was done asynchonously, we must
                  ** reply the request
                  */
   if(!(ios2->ios2_Req.io_Flags & SANA2IOF_QUICK))
      ReplyMsg((struct Message *)ios2);
   else           /* otherwise just mark it as done */
      ios2->ios2_Req.io_Message.mn_Node.ln_Type = NT_REPLYMSG;
}
PUBLIC ASM SAVEDS VOID DevBeginIO(REG(a1,struct IOSana2Req *ios2), REG(a6,BASEPTR))
{
   ULONG mtu;
   
      /* mark request as active */
   ios2->ios2_Req.io_Message.mn_Node.ln_Type = NT_MESSAGE;
   ios2->ios2_Req.io_Error = S2ERR_NO_ERROR;
   ios2->ios2_WireError = S2WERR_GENERIC_ERROR;

   d(("cmd = %ld\n",ios2->ios2_Req.io_Command));

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
   switch(ios2->ios2_Req.io_Command)
   {
      case CMD_READ:
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
            AddTail((struct List*)&pb->pb_ReadList, (struct Node*)ios2);
            ReleaseSemaphore(&pb->pb_ReadListSem);
            ios2 = NULL;
         }
      break;

      case S2_BROADCAST:
              /* set broadcast addr: ff:ff:ff:ff:ff:ff */
         memset(ios2->ios2_DstAddr, 0xff, HW_ADDRFIELDSIZE);
              /* fall through */
      case CMD_WRITE:
              /* determine max valid size */
         mtu = pb->pb_MTU;
         if(ios2->ios2_Req.io_Flags & SANA2IOF_RAW) {
            mtu += HW_ETH_HDR_SIZE;
         }
         if(ios2->ios2_DataLength > mtu)
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
            AddTail((struct List*)&pb->pb_WriteList, (struct Node*)ios2);
            ReleaseSemaphore(&pb->pb_WriteListSem);
            Signal((struct Task*)pb->pb_Server, SIGBREAKF_CTRL_F);
            ios2 = NULL;
         }
      break;

      case S2_ONLINE:
      case S2_OFFLINE:
      case S2_CONFIGINTERFACE:   /* forward request */
         DevForwardIO(pb, ios2);
         ios2 = NULL;
      break;

      case S2_GETSTATIONADDRESS:
         memcpy(ios2->ios2_SrcAddr, pb->pb_CfgAddr, HW_ADDRFIELDSIZE); /* current */
         memcpy(ios2->ios2_DstAddr, pb->pb_DefAddr, HW_ADDRFIELDSIZE); /* default */
      break;
         
      case S2_DEVICEQUERY:
      {
         struct Sana2DeviceQuery *devquery;

         devquery = ios2->ios2_StatData;
         devquery->DevQueryFormat = 0;        /* "this is format 0" */
         devquery->DeviceLevel = 0;           /* "this spec defines level 0" */
         
         if (devquery->SizeAvailable >= 18) devquery->AddrFieldSize = HW_ADDRFIELDSIZE * 8; /* Bits! */
         if (devquery->SizeAvailable >= 22) devquery->MTU           = pb->pb_MTU;
         if (devquery->SizeAvailable >= 26) devquery->BPS           = pb->pb_BPS;
         if (devquery->SizeAvailable >= 30) devquery->HardwareType  = S2WireType_Ethernet;
         
         devquery->SizeSupplied = min((int)devquery->SizeAvailable, 30);
      }
      break;
         
      case S2_ONEVENT:
         /* Two special cases. S2EVENT_ONLINE and S2EVENT_OFFLINE are supposed to
            retun immediately if we are already in the state that they are waiting
            for. */
         if (((ios2->ios2_WireError & S2EVENT_ONLINE) && !(pb->pb_Flags & PLIPF_OFFLINE)) ||
             ((ios2->ios2_WireError & S2EVENT_OFFLINE) && (pb->pb_Flags & PLIPF_OFFLINE)))
         {
            ios2->ios2_Req.io_Error = 0;
            ios2->ios2_WireError &= (S2EVENT_ONLINE|S2EVENT_OFFLINE);
            DevTermIO(pb,ios2);
            ios2 = NULL;
         }
         else if ((ios2->ios2_WireError & (S2EVENT_ERROR|S2EVENT_TX|S2EVENT_RX|S2EVENT_ONLINE|
                               S2EVENT_OFFLINE|S2EVENT_BUFF|S2EVENT_HARDWARE|S2EVENT_SOFTWARE))
                  != ios2->ios2_WireError)
         {
            /* we cannot handle such events */
            ios2->ios2_Req.io_Error = S2ERR_NOT_SUPPORTED;
            ios2->ios2_WireError = S2WERR_BAD_EVENT;
         }
         else
         {
            /* Queue anything else */
            ios2->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
            ObtainSemaphore(&pb->pb_EventListSem);
            AddTail((struct List*)&pb->pb_EventList, (struct Node*)ios2);
            ReleaseSemaphore(&pb->pb_EventListSem);
            ios2 = NULL;
         }
      break;

      /* --------------- stats support ----------------------- */

      case S2_TRACKTYPE:
         if (!addtracktype(pb, ios2->ios2_PacketType))
         {
            ios2->ios2_Req.io_Error = S2ERR_NO_RESOURCES;
         }
      break;

      case S2_UNTRACKTYPE:
         if (!remtracktype(pb, ios2->ios2_PacketType))
         {
            ios2->ios2_Req.io_Error = S2ERR_BAD_STATE;
            ios2->ios2_WireError = S2WERR_NOT_TRACKED;
         }
      break;

      case S2_GETTYPESTATS:
         if (!gettrackrec(pb, ios2->ios2_PacketType, (struct Sana2PacketTypeStats *)ios2->ios2_StatData))
         {
            ios2->ios2_Req.io_Error = S2ERR_BAD_STATE;
            ios2->ios2_WireError = S2WERR_NOT_TRACKED;
         }
      break;

      case S2_READORPHAN:
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
         {                       /* Enqueue it to the orphan-reader-list */
            ios2->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
            ObtainSemaphore(&pb->pb_ReadOrphanListSem);
            AddTail((struct List*)&pb->pb_ReadOrphanList, (struct Node*)ios2);
            ReleaseSemaphore(&pb->pb_ReadOrphanListSem);
            ios2 = NULL;
         }
      break;

      case S2_GETGLOBALSTATS:
         memcpy(ios2->ios2_StatData, &pb->pb_DevStats, sizeof(struct Sana2DeviceStats));
      break;

      case S2_GETSPECIALSTATS:
      {
         struct Sana2SpecialStatHeader *s2ssh = (struct Sana2SpecialStatHeader *)ios2->ios2_StatData;

         if (pb->pb_ExtFlags & PLIPEF_NOSPECIALSTATS)
         {
            s2ssh->RecordCountSupplied = 0;
         }
         else
         {
            s2ssh->RecordCountSupplied = s2ssh->RecordCountMax > S2SS_COUNT ?
                                          S2SS_COUNT : s2ssh->RecordCountMax;
            CopyMem(pb->pb_SpecialStats, (void*)(s2ssh+1),
                        s2ssh->RecordCountSupplied*sizeof(struct Sana2SpecialStatRecord));
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
         ios2->ios2_Req.io_Error = IOERR_NOCMD;
         ios2->ios2_WireError = S2WERR_GENERIC_ERROR;
      break;

         /* other commands (SANA-2) we don't support */
      /*case S2_ADDMULTICASTADDRESS:*/
      /*case S2_DELMULTICASTADDRESS:*/
      /*case S2_MULTICAST:*/
      default:
         ios2->ios2_Req.io_Error = S2ERR_NOT_SUPPORTED;
         ios2->ios2_WireError = S2WERR_GENERIC_ERROR;
      break;
   }

   if (ios2) DevTermIO(pb, ios2);

   return;
}

   /*
   ** stop io-command
   */
PUBLIC ASM SAVEDS LONG DevAbortIO(REG(a1,struct IOSana2Req *ior), REG(a6,BASEPTR))
{
   BOOL is;
   LONG rc = 0;

   d(("cmd = %ld\n",ior->ios2_Req.io_Command));

   ObtainSemaphore(&pb->pb_WriteListSem);
   if ((is = isinlist((struct Node*)ior, (struct List*)&pb->pb_WriteList))) abort_req(pb,ior);
   ReleaseSemaphore(&pb->pb_WriteListSem);
   if (is) goto leave;

   ObtainSemaphore(&pb->pb_ReadListSem);
   if ((is = isinlist((struct Node*)ior, (struct List*)&pb->pb_ReadList))) abort_req(pb,ior);
   ReleaseSemaphore(&pb->pb_ReadListSem);
   if (is) goto leave;

   ObtainSemaphore(&pb->pb_EventListSem);
   if ((is = isinlist((struct Node*)ior, (struct List*)&pb->pb_EventList))) abort_req(pb,ior);
   ReleaseSemaphore(&pb->pb_EventListSem);
   if (is) goto leave;

   ObtainSemaphore(&pb->pb_ReadOrphanListSem);
   if ((is = isinlist((struct Node*)ior, (struct List*)&pb->pb_ReadOrphanList))) abort_req(pb,ior);
   ReleaseSemaphore(&pb->pb_ReadOrphanListSem);
   if (is) goto leave;

   rc = -1;

leave:
   return rc;
}


#ifdef __GNUC__
extern void __restore_a4(void)
{
    __asm volatile("\tlea ___a4_init, a4");
}
#endif

