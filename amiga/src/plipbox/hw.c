/*
 * hw.c - hardware dependent part of driver
 */

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/cia.h>
#include <proto/misc.h>
#include <proto/timer.h>
#include <proto/utility.h>

#include <exec/memory.h>
#include <exec/interrupts.h>
#include <exec/devices.h>
#include <exec/io.h>

#include <devices/sana2.h>
#include <hardware/cia.h>
#include <resources/misc.h>

#include <string.h>

#include "global.h"
#include "debug.h"
#include "compiler.h"
#include "hw.h"
#include "hwbase.h"

/* magic packet types */
#define HW_MAGIC_ONLINE    0xffff
#define HW_MAGIC_OFFLINE   0xfffe
#define HW_MAGIC_LOOPBACK  0xfffd

/* externs in asm code */
GLOBAL VOID ASM interrupt(REG(a1,struct HWBase *hwb));
GLOBAL USHORT ASM CRC16(REG(a0,UBYTE *), REG(d0,SHORT));
GLOBAL BOOL ASM hwsend(REG(a0,struct HWBase *hwb), REG(a1,struct HWFrame *frame));
GLOBAL BOOL ASM hwrecv(REG(a0,struct HWBase *hwb), REG(a1,struct HWFrame *frame));
GLOBAL BOOL ASM hwburstsend(REG(a0,struct HWBase *), REG(a1,struct HWFrame *));
GLOBAL BOOL ASM hwburstrecv(REG(a0,struct HWBase *), REG(a1,struct HWFrame *));

   /* amiga.lib provides for these symbols */
extern FAR volatile struct CIA ciaa,ciab;

PRIVATE ULONG ASM SAVEDS exceptcode(REG(d0,ULONG sigmask), REG(a1,struct PLIPBase *hwb));

/* CIA access macros & functions */
#define CLEARINT        SetICR(CIAABase, CIAICRF_FLG)
#define DISABLEINT      AbleICR(CIAABase, CIAICRF_FLG)
#define ENABLEINT       AbleICR(CIAABase, CIAICRF_FLG | CIAICRF_SETCLR)

#define HS_RAK_MASK     CIAF_PRTRBUSY
#define HS_REQ_MASK     CIAF_PRTRPOUT

#define SETCIAOUTPUT    ciab.ciapra |= CIAF_PRTRSEL; ciaa.ciaddrb = 0xFF
#define SETCIAINPUT     ciab.ciapra &= ~CIAF_PRTRSEL; ciaa.ciaddrb = 0x00
#define PARINIT(b)      SETCIAINPUT; \
                        ciab.ciaddra &= ~HS_RAK_MASK; \
                        ciab.ciaddra |= HS_REQ_MASK | CIAF_PRTRSEL
#define PAREXIT \
                        ciab.ciaddra &= ~(CIAF_PRTRSEL | CIAF_PRTRBUSY | CIAF_PRTRPOUT); \
                        ciab.ciapra  &= ~(CIAF_PRTRSEL | CIAF_PRTRBUSY | CIAF_PRTRPOUT)
#define TESTLINE(b)     (ciab.ciapra & HS_RAK_MASK)
#define SETREQUEST(b)   ciab.ciapra |= HS_REQ_MASK
#define CLEARREQUEST(b) ciab.ciapra &= ~HS_REQ_MASK

/* magic packet to tell plipbox firmware we go online and our MAC */
static REGARGS BOOL hw_send_magic_pkt(struct PLIPBase *pb, USHORT magic)
{
   BOOL rc;

   struct HWFrame *frame = pb->pb_Frame;
   
   frame->hwf_Size = HW_ETH_HDR_SIZE;
   memcpy(frame->hwf_SrcAddr, pb->pb_CfgAddr, HW_ADDRFIELDSIZE);
   memset(frame->hwf_DstAddr, 0, HW_ADDRFIELDSIZE);
   frame->hwf_DstAddr[0] = DEVICE_VERSION;
   frame->hwf_DstAddr[1] = DEVICE_REVISION;
   frame->hwf_Type = magic;
   
   rc = hw_send_frame(pb, frame) ? TRUE : FALSE;
   return rc;
}

GLOBAL REGARGS void hw_get_sys_time(struct PLIPBase *pb, struct timeval *time)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
  GetSysTime(time);
}

#define PLIP_DEFTIMEOUT          (500*1000)
#define PLIP_MINTIMEOUT          500
#define PLIP_MAXTIMEOUT          (10000*1000)

GLOBAL REGARGS void hw_config_init(struct PLIPBase *pb,
                                   STRPTR *template_str,
                                   struct CommonConfig **cfg,
                                   STRPTR *config_file)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;

  hwb->hwb_TimeOutSecs = PLIP_DEFTIMEOUT / 1000000L;
  hwb->hwb_TimeOutMicros = PLIP_DEFTIMEOUT % 1000000L;
  hwb->hwb_BurstMode = 1;

  *template_str = (STRPTR)TEMPLATE;
  *config_file = (STRPTR)CONFIGFILE;
  *cfg = (struct CommonConfig *)&hwb->hwb_Config;
}

GLOBAL REGARGS void hw_config_update(struct PLIPBase *pb)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
  struct TemplateConfig *args = &hwb->hwb_Config;

  if (args->timeout) {
    LONG to = BOUNDS(*args->timeout, PLIP_MINTIMEOUT, PLIP_MAXTIMEOUT);
    hwb->hwb_TimeOutMicros = to % 1000000L;
    hwb->hwb_TimeOutSecs = to / 1000000L;
  }

  if(args->no_burst) {
    hwb->hwb_BurstMode = 0;
  }
}

GLOBAL REGARGS void hw_config_dump(struct PLIPBase *pb)
{
#if DEBUG & 1
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
#endif
  d(("timeOut %ld.%ld\n", hwb->hwb_TimeOutSecs, hwb->hwb_TimeOutMicros));
  d(("burstMode %ld\n", (ULONG)hwb->hwb_BurstMode));
}

GLOBAL REGARGS BOOL hw_init(struct PLIPBase *pb)
{
   BOOL rc = FALSE;
   struct HWBase *hwb = (struct HWBase *)AllocVec(sizeof(struct HWBase), MEMF_CLEAR|MEMF_ANY);
   pb->pb_HWBase = hwb;
   if(hwb == NULL) {
     d(("no memory!\n"));
     return FALSE;
   }
   
   /* clone sys base, process */
   hwb->hwb_SysBase = pb->pb_SysBase;
   hwb->hwb_Server = pb->pb_Server;
   hwb->hwb_MaxFrameSize = (UWORD)pb->pb_MTU + HW_ETH_HDR_SIZE;
   d2(("hwb=%08lx, sysbase=%08lx, server=%08lx, hwb=%08lx, maxFrameSize=%ld\n",
      hwb, hwb->hwb_SysBase, hwb->hwb_Server, hwb, (ULONG)hwb->hwb_MaxFrameSize));
   
   if ((hwb->hwb_IntSig = AllocSignal(-1)) != -1)
   {
      hwb->hwb_IntSigMask = 1L << hwb->hwb_IntSig;
      d2(("int sigmask=%08lx\n",hwb->hwb_IntSigMask));
   
      if ((hwb->hwb_TimeoutPort = CreateMsgPort()))
      {
         ULONG sigmask;
         struct Process *proc = pb->pb_Server;
         
         /* save old exception setup */
         hwb->hwb_OldExcept = SetExcept(0, 0xffffffff); /* turn'em off */
         hwb->hwb_OldExceptCode = proc->pr_Task.tc_ExceptCode;
         hwb->hwb_OldExceptData = proc->pr_Task.tc_ExceptData;

         /* create new exception setup */
         proc->pr_Task.tc_ExceptCode = (APTR)&exceptcode;
         proc->pr_Task.tc_ExceptData = (APTR)pb;
         sigmask = 1 << hwb->hwb_TimeoutPort->mp_SigBit;
         SetSignal(0, sigmask);
         SetExcept(sigmask, sigmask);

         /* enter port address */
         hwb->hwb_TimeoutReq.tr_node.io_Message.mn_ReplyPort = hwb->hwb_TimeoutPort;
         if (!OpenDevice((STRPTR)"timer.device", UNIT_MICROHZ, (struct IORequest*)&hwb->hwb_TimeoutReq, 0))
         {
             TimerBase = (struct Library *)hwb->hwb_TimeoutReq.tr_node.io_Device;

             /* setup the timeout stuff */
             hwb->hwb_TimeoutReq.tr_node.io_Flags = IOF_QUICK;
             hwb->hwb_TimeoutReq.tr_node.io_Command = TR_ADDREQUEST;
             hwb->hwb_TimeoutSet = 0xff;

             rc = TRUE;
          }
          else
          {
             d(("couldn't open timer.device"));
          }
       }
       else
       {
          d(("no port for timeout handling\n"));
       }
    } 
    else 
    {
       d(("no interrupt signal\n",rc));
    }
    
    return rc;              
}

GLOBAL REGARGS VOID hw_cleanup(struct PLIPBase *pb)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   
   if (hwb->hwb_TimeoutPort)
   {
      struct Process *proc = pb->pb_Server;
      
      /* restore old exception setup */
      SetExcept(0, 0xffffffff);    /* turn'em off */
      proc->pr_Task.tc_ExceptCode = hwb->hwb_OldExceptCode;
      proc->pr_Task.tc_ExceptData = hwb->hwb_OldExceptData;
      SetExcept(hwb->hwb_OldExcept, 0xffffffff);

      if (TimerBase)
      {
         WaitIO((struct IORequest*)&hwb->hwb_TimeoutReq);
         CloseDevice((struct IORequest*)&hwb->hwb_CollReq);
      }
      DeleteMsgPort(hwb->hwb_TimeoutPort);
   }
   
   if (hwb->hwb_IntSig != -1) {
      FreeSignal(hwb->hwb_IntSig);
   }

   FreeVec(hwb);
   pb->pb_HWBase = NULL;
}

/*
 * hwattach - setup hardware if device gets online
 */
GLOBAL REGARGS BOOL hw_attach(struct PLIPBase *pb)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   BOOL rc = FALSE;

   d(("hw_attach entered, hwb=%08lx\n", hwb));

   hwb->hwb_AllocFlags = 0;
   if ((MiscBase = OpenResource((STRPTR)"misc.resource")) != NULL)
   {
      if ((CIAABase = OpenResource((STRPTR)"ciaa.resource")) != NULL)
      {
         CiaBase = CIAABase;

         d(("ciabase is %lx\n",CiaBase));

         /* obtain exclusive access to the parallel hardware */
         if (!AllocMiscResource(MR_PARALLELPORT, (STRPTR)pb->pb_DevNode.lib_Node.ln_Name))
         {
            hwb->hwb_AllocFlags |= 1;
            if (!AllocMiscResource(MR_PARALLELBITS, (STRPTR)pb->pb_DevNode.lib_Node.ln_Name))
            {
               hwb->hwb_AllocFlags |= 2;

               /* Add our interrupt to handle CIAICRB_FLG.
               ** This is also cia.resource means of granting exclusive
               ** access to the related registers in the CIAs.
               */
               hwb->hwb_Interrupt.is_Node.ln_Type = NT_INTERRUPT;
               hwb->hwb_Interrupt.is_Node.ln_Pri  = 127;
               hwb->hwb_Interrupt.is_Node.ln_Name = SERVERTASKNAME;
               hwb->hwb_Interrupt.is_Data         = (APTR)hwb;
               hwb->hwb_Interrupt.is_Code         = (VOID (*)())&interrupt;

               d(("interrupt @%08lx\n",&interrupt));

               /* We must Disable() bcos there could be an interrupt already
               ** waiting for us. We may, however, not Able/SetICR() before
               ** we have access!
               */
               Disable();
               if (!AddICRVector(CIAABase, CIAICRB_FLG, &hwb->hwb_Interrupt))
               {
                  DISABLEINT;                       /* this is what I meant */
                  rc = TRUE;
               }
               Enable();

               if (rc)
               {
                  hwb->hwb_AllocFlags |= 4;
                  PARINIT(pb);    /* cia to input, handshake in/out setting */
                  CLEARREQUEST(pb);                /* setup handshake lines */
                  CLEARINT;                         /* clear this interrupt */
                  ENABLEINT;                            /* allow interrupts */
               }

            }
            else
               d(("no parallelbits\n"));
         }
         else
            d(("no parallelport\n"));
      }
      else
         d(("no misc resource\n"));
   }
   else
      d(("no misc resource\n"));

   /* finally send magic packet to tell mcu to go online */
   if(rc) {
      d(("send magic packet %04lu\n", HW_MAGIC_ONLINE));
      rc = hw_send_magic_pkt(pb, HW_MAGIC_ONLINE);
   }

   return rc;
}

/*
 * shutdown hardware if device gets offline
 */
GLOBAL REGARGS VOID hw_detach(struct PLIPBase *pb)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   
   /* first tell mcu to go offline */
   hw_send_magic_pkt(pb, HW_MAGIC_OFFLINE);

   if (hwb->hwb_AllocFlags & 4)
   {
      PAREXIT;
      DISABLEINT;
      CLEARINT;
      RemICRVector(CIAABase, CIAICRB_FLG, &hwb->hwb_Interrupt);
   }

   if (hwb->hwb_AllocFlags & 2) FreeMiscResource(MR_PARALLELBITS);

   if (hwb->hwb_AllocFlags & 1) FreeMiscResource(MR_PARALLELPORT);

   hwb->hwb_AllocFlags = 0;
}

PRIVATE ULONG ASM SAVEDS exceptcode(REG(d0,ULONG sigmask), REG(a1,struct PLIPBase *pb))
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   
   d8(("+ex\n"));
   
   /*extern void KPrintF(char *,...);
   KPrintF("exceptcode\n");*/

   /* remove the I/O Block from the port */
   WaitIO((struct IORequest*)&hwb->hwb_TimeoutReq);

   /* this tells the xfer routines to cease polling */
   hwb->hwb_TimeoutSet = 0xff;
   
   d8(("-ex\n"));
   return sigmask;            /* re-enable the signal */
}

GLOBAL REGARGS BOOL hw_send_frame(struct PLIPBase *pb, struct HWFrame *frame)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   BOOL rc;

   /* wait until I/O block is safe to be reused */
   while(!hwb->hwb_TimeoutSet) Delay(1L);
   
   /* start new timeout timer */
   hwb->hwb_TimeoutReq.tr_time.tv_secs = hwb->hwb_TimeOutSecs;
   hwb->hwb_TimeoutReq.tr_time.tv_micro = hwb->hwb_TimeOutMicros;
   hwb->hwb_TimeoutSet = 0;
   SendIO((struct IORequest*)&hwb->hwb_TimeoutReq);

   /* hw send */
   if(hwb->hwb_BurstMode) {
     d8(("+txb\n"));
     rc = hwburstsend(hwb, frame);
   } else {
     d8(("+tx\n"));
     rc = hwsend(hwb, frame);
   }
   d8(("-tx: %s\n", rc ? "ok":"ERR"));
      
   /* stop timeout timer */ 
   AbortIO((struct IORequest*)&hwb->hwb_TimeoutReq);
   
   return rc;
}

GLOBAL REGARGS BOOL hw_recv_pending(struct PLIPBase *pb)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   if ((hwb->hwb_Flags & HWF_RECV_PENDING) == HWF_RECV_PENDING) 
   {
      return TRUE;
   } 
   else {
      return FALSE;
   }
}

GLOBAL REGARGS BOOL hw_recv_frame(struct PLIPBase *pb, struct HWFrame *frame)
{
   ULONG pkttyp;
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   BOOL rc = TRUE;

   while(1) {
      /* wait until I/O block is safe to be reused */
      while(!hwb->hwb_TimeoutSet) Delay(1L);

      /* start new timeout timer */
      hwb->hwb_TimeoutReq.tr_time.tv_secs    = hwb->hwb_TimeOutSecs;
      hwb->hwb_TimeoutReq.tr_time.tv_micro   = hwb->hwb_TimeOutMicros;
      hwb->hwb_TimeoutSet = 0;
      SendIO((struct IORequest*)&hwb->hwb_TimeoutReq);

      /* hw recv */
      if(hwb->hwb_BurstMode) {
        d8(("+rxb\n"));
        rc = hwburstrecv(hwb, frame);
      } else {
        d8(("+rx\n"));
        rc = hwrecv(hwb, frame);
      }
      d8(("+rx: %s\n", rc ? "ok":"ERR"));

      /* stop timeout timer */
      AbortIO((struct IORequest*)&hwb->hwb_TimeoutReq);

      /* error? */
      if(!rc) {
         break;
      }

      /* perform internal loop back of magic packets of type 0xfffd */
      pkttyp = frame->hwf_Type;
      if(pkttyp == HW_MAGIC_LOOPBACK) {
         d(("loop back packet (size %ld)\n",frame->hwf_Size));
         rc = hw_send_frame(pb, frame);
      }
      /* plipbox requests online magic (again) */
      else if(pkttyp == HW_MAGIC_ONLINE) {
         d(("request online magic"));
         rc = hw_send_magic_pkt(pb, HW_MAGIC_ONLINE);
      }
      else {
         break;
      }
   }
   return rc;
}

GLOBAL REGARGS ULONG hw_recv_sigmask(struct PLIPBase *pb)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   return hwb->hwb_IntSigMask | hwb->hwb_CollSigMask;
}
