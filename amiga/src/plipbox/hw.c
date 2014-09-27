/*
 * hw.c - hardware dependent part of driver
 */

#define DEBUG 0

/*F*/ /* includes */
#ifndef CLIB_EXEC_PROTOS_H
#include <clib/exec_protos.h>
#include <pragmas/exec_sysbase_pragmas.h>
#endif
#ifndef CLIB_DOS_PROTOS_H
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
#endif
#ifndef CLIB_CIA_PROTOS_H
#include <clib/cia_protos.h>
#include <pragmas/cia_pragmas.h>
#endif
#ifndef CLIB_MISC_PROTOS_H
#include <clib/misc_protos.h>
#include <pragmas/misc_pragmas.h>
#endif
#ifndef CLIB_TIME_PROTOS_H
#include <clib/timer_protos.h>
#include <pragmas/timer_pragmas.h>
#endif
#ifndef CLIB_UTILITY_PROTOS_H
#include <clib/utility_protos.h>
#include <pragmas/utility_pragmas.h>
#endif

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif
#ifndef EXEC_INTERRUPTS_H
#include <exec/interrupts.h>
#endif
#ifndef EXEC_DEVICES_H
#include <exec/devices.h>
#endif
#ifndef EXEC_IO_H
#include <exec/io.h>
#endif

#ifndef DEVICES_SANA2_H
#include <devices/sana2.h>
#endif

#ifndef HARDWARE_CIA_H
#include <hardware/cia.h>
#endif

#ifndef RESOURCES_MISC_H
#include <resources/misc.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef __GLOBAL_H
#include "global.h"
#endif
#ifndef __DEBUG_H
#include "debug.h"
#endif
#ifndef __COMPILER_H
#include "compiler.h"
#endif
#ifndef __HW_H
#include "hw.h"
#endif
/*E*/

/* externs in asm code */
GLOBAL VOID ASM interrupt(REG(a1) struct HWB *hwb);
GLOBAL USHORT ASM CRC16(REG(a0) UBYTE *, REG(d0) SHORT);
GLOBAL BOOL ASM hwsend(REG(a0) struct HWBase *hwb, REG(a1) struct HWFrame *frame);
GLOBAL BOOL ASM hwrecv(REG(a0) struct HWBase *hwb, REG(a1) struct HWFrame *frame);
   /* amiga.lib provides for these symbols */
GLOBAL FAR volatile struct CIA ciaa,ciab;

PRIVATE ULONG ASM SAVEDS exceptcode(REG(d0) ULONG sigmask, REG(a1) struct PLIPBase *hwb);

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

#define MAGIC_ONLINE    0xffff
#define MAGIC_OFFLINE   0xfffe

/* magic packet to tell plipbox firmware we go online and our MAC */
PRIVATE REGARGS BOOL send_magic_pkt(BASEPTR, USHORT magic)
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

#define PLIP_DEFTIMEOUT          (5000*1000)
#define PLIP_MINTIMEOUT          500
#define PLIP_MAXTIMEOUT          (10000*1000)

GLOBAL void hw_config_init(struct PLIPBase *pb)
{
  struct HWBase *hwb = &pb->pb_HWBase;

  hwb->hwb_TimeOutSecs = PLIP_DEFTIMEOUT / 1000000L;
  hwb->hwb_TimeOutMicros = PLIP_DEFTIMEOUT % 1000000L; 
}

GLOBAL void hw_config_update(struct PLIPBase *pb, struct TemplateConfig *args)
{
  struct HWBase *hwb = &pb->pb_HWBase;
  
  if (args->timeout) {
    LONG to = BOUNDS(*args->timeout, PLIP_MINTIMEOUT, PLIP_MAXTIMEOUT);
    hwb->hwb_TimeOutMicros = to % 1000000L;
    hwb->hwb_TimeOutSecs = to / 1000000L;
  }
}

GLOBAL void hw_config_dump(struct PLIPBase *pb)
{
#if DEBUG & 1
  struct HWBase *hwb = &pb->pb_HWBase;
#endif
  d(("timeOut %ld\n", hwb->hwb_TimeOut));
}

GLOBAL BOOL hw_init(struct PLIPBase *pb)
{
   struct HWBase *hwb = &pb->pb_HWBase;
   
   BOOL rc = FALSE;
   
   /* clone sys base, process */
   hwb->hwb_SysBase = pb->pb_SysBase;
   hwb->hwb_Server = pb->pb_Server;
   hwb->hwb_MaxMTU = HW_ETH_MTU;
   d2(("sysbase=%08lx, server=%08lx, hwb=%08lx, MTU=%d\n",
      hwb->hwb_SysBase, hwb->hwb_Server, hwb, hwb->hwb_MaxMTU));
   
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
         if (!OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest*)&hwb->hwb_TimeoutReq, 0))
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

GLOBAL VOID hw_cleanup(struct PLIPBase *pb)
{
   struct HWBase *hwb = &pb->pb_HWBase;
   
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
}

/*
 * hwattach - setup hardware if device gets online
 */
GLOBAL BOOL hw_attach(struct PLIPBase *pb)
{
   struct HWBase *hwb = &pb->pb_HWBase;
   BOOL rc = FALSE;

   d(("entered\n"));

   hwb->hwb_AllocFlags = 0;
   if (MiscBase = OpenResource("misc.resource"))
   {
      if (CIAABase = OpenResource("ciaa.resource"))
      {
         CiaBase = CIAABase;

         d(("ciabase is %lx\n",CiaBase));

         /* obtain exclusive access to the parallel hardware */
         if (!AllocMiscResource(MR_PARALLELPORT, pb->pb_DevNode.lib_Node.ln_Name))
         {
            hwb->hwb_AllocFlags |= 1;
            if (!AllocMiscResource(MR_PARALLELBITS, pb->pb_DevNode.lib_Node.ln_Name))
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

                  /* send magic */
                  rc = send_magic_pkt(pb, MAGIC_ONLINE);
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

   return rc;
}

/*
 * shutdown hardware if device gets offline
 */
GLOBAL VOID hw_detach(struct PLIPBase *pb)
{
   struct HWBase *hwb = &pb->pb_HWBase;
   
   /* send magic */
   send_magic_pkt(pb, MAGIC_OFFLINE);

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

PRIVATE ULONG ASM SAVEDS exceptcode(REG(d0) ULONG sigmask, REG(a1) struct PLIPBase *pb)
{
   struct HWBase *hwb = &pb->pb_HWBase;
   
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

GLOBAL BOOL hw_send_frame(struct PLIPBase *pb, struct HWFrame *frame)
{
   struct HWBase *hwb = &pb->pb_HWBase;
   BOOL rc;

   /* wait until I/O block is safe to be reused */
   while(!hwb->hwb_TimeoutSet) Delay(1L);
   
   /* start new timeout timer */
   hwb->hwb_TimeoutReq.tr_time.tv_secs = hwb->hwb_TimeOutSecs;
   hwb->hwb_TimeoutReq.tr_time.tv_micro = hwb->hwb_TimeOutMicros;
   hwb->hwb_TimeoutSet = 0;
   SendIO((struct IORequest*)&hwb->hwb_TimeoutReq);

   /* hw send */
   d8(("+tx\n"));
   rc = hwsend(hwb, frame);
   d8(("-tx: %s\n", rc ? "ok":"ERR"));
      
   /* stop timeout timer */ 
   AbortIO((struct IORequest*)&hwb->hwb_TimeoutReq);
   
   return rc;
}

GLOBAL BOOL hw_recv_pending(struct PLIPBase *pb)
{
   struct HWBase *hwb = &pb->pb_HWBase;
   if ((hwb->hwb_Flags & HWF_RECV_PENDING) == HWF_RECV_PENDING) 
   {
      return TRUE;
   } 
   else {
      return FALSE;
   }
}

GLOBAL BOOL hw_recv_frame(struct PLIPBase *pb, struct HWFrame *frame)
{
   struct HWBase *hwb = &pb->pb_HWBase;
   BOOL rc;

   /* wait until I/O block is safe to be reused */
   while(!hwb->hwb_TimeoutSet) Delay(1L);
    
   /* start new timeout timer */
   hwb->hwb_TimeoutReq.tr_time.tv_secs    = hwb->hwb_TimeOutSecs;
   hwb->hwb_TimeoutReq.tr_time.tv_micro   = hwb->hwb_TimeOutMicros;
   hwb->hwb_TimeoutSet = 0;
   SendIO((struct IORequest*)&hwb->hwb_TimeoutReq);

   /* hw recv */
   d8(("+rx\n"));
   rc = hwrecv(hwb, frame);
   d8(("+rx: %s\n", rc ? "ok":"ERR"));
    
   /* stop timeout timer */
   AbortIO((struct IORequest*)&hwb->hwb_TimeoutReq);
   
   return rc;
}

GLOBAL ULONG hw_recv_sigmask(struct PLIPBase *pb)
{
   struct HWBase *hwb = &pb->pb_HWBase;
   return hwb->hwb_IntSigMask | hwb->hwb_CollSigMask;
}
