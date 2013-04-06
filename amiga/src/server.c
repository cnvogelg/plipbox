/*
** $VER: server.c 1.15 (01 Apr 1998)
**
** magplip.device - Parallel Line Internet Protocol
**
** Original code written by Oliver Wagner and Michael Balzer.
**
** This version has been completely reworked by Marius Gröger, introducing
** slight protocol changes. The new source is a lot better organized and
** maintainable.
**
** Additional changes and code cleanup by Jan Kratochvil and Martin Mares.
** The new source is significantly faster and yet better maintainable.
**
** (C) Copyright 1993-1994 Oliver Wagner & Michael Balzer
** (C) Copyright 1995 Jan Kratochvil & Martin Mares
** (C) Copyright 1995-1996 Marius Gröger
**     All Rights Reserved
**
** $HISTORY:
**
** 01 Apr 1998 : 001.015 :  integrated modifications for linPLIP from Stephane
** 19 Mar 1998 : 001.014 :  fixed S2_ONLINE bug, which returned an
**                          error by a call to go online if it was
**                          already online. Report from Holger Kruse.
**                          Fixed by Stefan Ruppert.
** 29 Mar 1996 : 001.014 :  changed copyright note
** 24 Feb 1996 : 001.013 :  added PRTRSEL data direction signal
** 30 Dec 1995 : 001.012 :  + dynamic allocation of only one frame buffer
**                          PLIP_MAXMTU now 128k
**                          + a bad MTU setting in ENV: will be
**                          forced to PLIP_MAXMTU instead of PLIP_DEFMTU
**                          + server task acknowledge now after calling
**                          readargs to avoid inconsistencies
** 03 Sep 1995 : 001.011 :  hardware addressing nicer
** 30 Aug 1995 : 001.010 :  + support for timer-timed timeout :-)
**                          + minor declaration related changes
** 20 Aug 1995 : 001.009 :  support for ASM xfer routines
**                          removed obsolete CIA macros (mag/jk/mm)
** 29 Jul 1995 : 001.008 :  support for arbitration delay
**                          symmetrical handling
** 26 Apr 1995 : 001.007 :  _very_ nasty bug would miss packets and get
**                          the driver totally irritated
** 25 Apr 1995 : 001.006 :  now compiles with ANSI and STRICT
**                          fixed bug with resource allocation
** 08 Mar 1995 : 001.005 :  write req. are now handled by device.c
** 06 Mar 1995 : 001.004 :  collision delay added
** 06 Mar 1995 : 001.003 :  hardware transmission errors are no longer retried
**                          because this is any upper layers job
** 04 Mar 1995 : 001.002 :  event tracking *much* more conform to SANA-2
** 18 Feb 1995 : 001.001 :  startup now a bit nicer
**                          using BASEPTR
** 12 Feb 1995 : 001.000 :  reworked original
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

#ifndef __MAGPLIP_H
#include "magplip.h"
#endif
#ifndef __DEBUG_H
#include "debug.h"
#endif
#ifndef __COMPILER_H
#include "compiler.h"
#endif
/*E*/

/*F*/ /* defines, types and enums */

   /*
   ** return codes for arbitratedwrite()
   */
typedef enum { AW_OK, AW_ABORTED, AW_BUFFER_ERROR, AW_ERROR } AW_RESULT;

   /* return val, cut to min or max if exceeding range */
#define BOUNDS(val, min, max) ((val) <= (max) ? ((val) >= (min) ? (val) :\
							 (min)) : (max))

/*E*/
/*F*/ /* imports */
   /* external functions */
GLOBAL VOID dotracktype(BASEPTR, ULONG type, ULONG ps, ULONG pr, ULONG bs, ULONG br, ULONG pd);
GLOBAL VOID DevTermIO(BASEPTR, struct IOSana2Req *ios2);
GLOBAL USHORT ASM CRC16(REG(a0) UBYTE *, REG(d0) SHORT);
GLOBAL BOOL ASM hwsend(REG(a0) BASEPTR);
GLOBAL BOOL ASM hwrecv(REG(a0) BASEPTR);
GLOBAL VOID ASM interrupt(REG(a1) BASEPTR);

   /* amiga.lib provides for these symbols */
GLOBAL FAR volatile struct CIA ciaa,ciab;
/*E*/
/*F*/ /* exports */
PUBLIC VOID SAVEDS ServerTask(void);
/*E*/
/*F*/ /* private */
PRIVATE struct PLIPBase *startup(void);
PRIVATE REGARGS VOID DoEvent(BASEPTR, long event);
PRIVATE VOID readargs(BASEPTR);
PRIVATE BOOL init(BASEPTR);
PRIVATE BOOL hwattach(BASEPTR);
PRIVATE VOID hwdetach(BASEPTR);
PRIVATE REGARGS BOOL goonline(BASEPTR);
PRIVATE REGARGS VOID gooffline(BASEPTR);
PRIVATE REGARGS AW_RESULT arbitratedwrite(BASEPTR, struct IOSana2Req *ios2);
PRIVATE REGARGS VOID dowritereqs(BASEPTR);
PRIVATE REGARGS VOID doreadreqs(BASEPTR);
PRIVATE REGARGS VOID dos2reqs(BASEPTR);
/*E*/

/*F*/ /* CIA access macros & functions */

#define CLEARINT        SetICR(CIAABase, CIAICRF_FLG)
#define DISABLEINT      AbleICR(CIAABase, CIAICRF_FLG)
#define ENABLEINT       AbleICR(CIAABase, CIAICRF_FLG | CIAICRF_SETCLR)

#ifdef LINPLIP

/* Most of these defines have no meaning at all, but it's safe ... */
#  define SETCIAOUTPUT
#  define SETCIAINPUT
#  define PARINIT(b)      ciaa.ciaddrb = 0xff; ciaa.ciaprb = 0x00
#  define PAREXIT

/* Good line : SELECT=POUT=PUSY=0 */
#  define TESTLINE(b)     ((ciab.ciapra&(CIAF_PRTRSEL|CIAF_PRTRPOUT|CIAF_PRTRBUSY)) || pb->pb_Flags&PLIPF_RECEIVING)
#  define SETREQUEST(b)
#  define CLEARREQUEST(b)

#else

#  define SETCIAOUTPUT    ciab.ciapra |= CIAF_PRTRSEL; ciaa.ciaddrb = 0xFF
#  define SETCIAINPUT     ciab.ciapra &= ~CIAF_PRTRSEL; ciaa.ciaddrb = 0x00
#  define PARINIT(b)      SETCIAINPUT;                                       \
			ciab.ciaddra &= ~((b)->pb_HandshakeMask[HS_LINE]); \
			ciab.ciaddra |= (b)->pb_HandshakeMask[HS_REQUEST] | CIAF_PRTRSEL
#  define PAREXIT \
			ciab.ciaddra &= ~(CIAF_PRTRSEL | CIAF_PRTRBUSY | CIAF_PRTRPOUT); \
			ciab.ciapra  &= ~(CIAF_PRTRSEL | CIAF_PRTRBUSY | CIAF_PRTRPOUT)
#  define TESTLINE(b)     (ciab.ciapra & (b)->pb_HandshakeMask[HS_LINE])
#  define SETREQUEST(b)   ciab.ciapra |= (b)->pb_HandshakeMask[HS_REQUEST]
#  define CLEARREQUEST(b) ciab.ciapra &= ~((b)->pb_HandshakeMask[HS_REQUEST])

#endif

/*E*/

   /*
   ** functions to gain/release hardware, initialise communication
   ** and for xmission timeout handling
   */
/*F*/ PRIVATE BOOL hwattach(BASEPTR)
{
   BOOL rc = FALSE;

   d(("entered\n"));

   if (MiscBase = OpenResource("misc.resource"))
   {
      if (CIAABase = OpenResource("ciaa.resource"))
      {
	 CiaBase = CIAABase;

	 d(("ciabase is %lx\n",CiaBase));

	 /* obtain exclusive access to the parallel hardware */
	 if (!AllocMiscResource(MR_PARALLELPORT, pb->pb_DevNode.lib_Node.ln_Name))
	 {
	    pb->pb_AllocFlags |= 1;
	    if (!AllocMiscResource(MR_PARALLELBITS, pb->pb_DevNode.lib_Node.ln_Name))
	    {
	       pb->pb_AllocFlags |= 2;

	       /* Add our interrupt to handle CIAICRB_FLG.
	       ** This is also cia.resource means of granting exclusive
	       ** access to the related registers in the CIAs.
	       */
	       pb->pb_Interrupt.is_Node.ln_Type = NT_INTERRUPT;
	       pb->pb_Interrupt.is_Node.ln_Pri  = 127;
	       pb->pb_Interrupt.is_Node.ln_Name = SERVERTASKNAME;
	       pb->pb_Interrupt.is_Data         = (APTR)pb;
	       pb->pb_Interrupt.is_Code         = (VOID (*)())&interrupt;

	       /* We must Disable() bcos there could be an interrupt already
	       ** waiting for us. We may, however, not Able/SetICR() before
	       ** we have access!
	       */
	       Disable();
	       if (!AddICRVector(CIAABase, CIAICRB_FLG, &pb->pb_Interrupt))
	       {
		  DISABLEINT;                       /* this is what I meant */
		  rc = TRUE;
	       }
	       Enable();

	       if (rc)
	       {
		  pb->pb_AllocFlags |= 4;
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

   return rc;
}
/*E*/
/*F*/ PRIVATE VOID hwdetach(BASEPTR)
{
   if (pb->pb_AllocFlags & 4)
   {
      PAREXIT;
      DISABLEINT;
      CLEARINT;
      RemICRVector(CIAABase, CIAICRB_FLG, &pb->pb_Interrupt);
   }

   if (pb->pb_AllocFlags & 2) FreeMiscResource(MR_PARALLELBITS);

   if (pb->pb_AllocFlags & 1) FreeMiscResource(MR_PARALLELPORT);

   pb->pb_AllocFlags = 0;
}
/*E*/
/*F*/ PRIVATE ULONG ASM SAVEDS exceptcode(REG(d0) ULONG sigmask, REG(a1) BASEPTR)
{
   /*extern void KPrintF(char *,...);
   KPrintF("exceptcode\n");*/

   /* remove the I/O Block from the port */
   WaitIO((struct IORequest*)&pb->pb_TimeoutReq);

   /* this tells the xfer routines to cease polling */
   pb->pb_TimeoutSet = 0xff;

   return sigmask;            /* re-enable the signal */
}
/*E*/

   /*
   ** functions to go online/offline
   */
/*F*/ PRIVATE REGARGS VOID rejectpackets(BASEPTR)
{
   struct IOSana2Req *ios2;

   ObtainSemaphore(&pb->pb_WriteListSem);
   while(ios2 = (struct IOSana2Req *)RemHead((struct List*)&pb->pb_WriteList))
   {
      ios2->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
      ios2->ios2_WireError = S2WERR_UNIT_OFFLINE;
      DevTermIO(pb,ios2);
   }
   ReleaseSemaphore(&pb->pb_WriteListSem);

   ObtainSemaphore(&pb->pb_ReadListSem);
   while(ios2 = (struct IOSana2Req *)RemHead((struct List*)&pb->pb_ReadList))
   {
      ios2->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
      ios2->ios2_WireError = S2WERR_UNIT_OFFLINE;
      DevTermIO(pb,ios2);
   }
   ReleaseSemaphore(&pb->pb_ReadListSem);

   ObtainSemaphore(&pb->pb_ReadOrphanListSem);
   while(ios2 = (struct IOSana2Req *)RemHead((struct List*)&pb->pb_ReadOrphanList))
   {
      ios2->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
      ios2->ios2_WireError = S2WERR_UNIT_OFFLINE;
      DevTermIO(pb,ios2);
   }
   ReleaseSemaphore(&pb->pb_ReadOrphanListSem);
}
/*E*/
/*F*/ PRIVATE REGARGS BOOL goonline(BASEPTR)
{
   BOOL rc = TRUE;

   d(("trying to go online\n"));

   if (pb->pb_Flags & (PLIPF_OFFLINE | PLIPF_NOTCONFIGURED))
   {
      if (!hwattach(pb))
      {
	 d(("error going online\n"));
	 rc = FALSE;
      }
      else
      {
	 GetSysTime(&pb->pb_DevStats.LastStart);
	 pb->pb_Flags &= ~(PLIPF_OFFLINE | PLIPF_NOTCONFIGURED);
	 DoEvent(pb, S2EVENT_ONLINE);
	 d(("i'm now online!\n"));
      }
   }

   return rc;
}
/*E*/
/*F*/ PRIVATE REGARGS VOID gooffline(BASEPTR)
{
   if (!(pb->pb_Flags & (PLIPF_OFFLINE | PLIPF_NOTCONFIGURED)))
   {
      hwdetach(pb);

      pb->pb_Flags |= PLIPF_OFFLINE;

      DoEvent(pb, S2EVENT_OFFLINE);
   }
   d(("ok!\n"));
}
/*E*/

   /*
   ** SANA-2 Event management
   */
/*F*/ PRIVATE REGARGS VOID DoEvent(BASEPTR, long event)
{
   struct IOSana2Req *ior, *ior2;

   d(("event is %lx\n",event));

   ObtainSemaphore(&pb->pb_EventListSem );
   
   for(ior = (struct IOSana2Req *) pb->pb_EventList.lh_Head;
       ior2 = (struct IOSana2Req *) ior->ios2_Req.io_Message.mn_Node.ln_Succ;
       ior = ior2 )
   {
      if (ior->ios2_WireError & event)
      {
	 Remove((struct Node*)ior);
	 DevTermIO(pb, ior);
      }
   }
   
   ReleaseSemaphore(&pb->pb_EventListSem );
}
/*E*/

   /*
   ** writing packets
   */
/*F*/ PRIVATE REGARGS AW_RESULT arbitratedwrite(BASEPTR, struct IOSana2Req *ios2)
{
   BOOL having_line;
   AW_RESULT rc;
   struct PLIPFrame *frame = pb->pb_Frame;

   /*
   ** Arbitration
   ** ===========================================================
   **
   ** Pseudo code of the arbitration:
   **
   **  if LINE is high (other side is ready to receive) then
   **     set REQUEST high (tell the other side we're ready to send)
   **     if LINE is high (other side is still ready to receive) then
   **        we have the line, do transfer
   **     reset REQUEST to low (tell other side we're ready to receive)
   **
   **    AW_OK             if we could transmit all the data correctly
   **    AW_BUFF_ERROR     if the BufferManagement callback failed
   **    AW_ERROR          if we got the line, but the actual transfer
   **                      failed, perhaps due to a timeout
   **    AW_ABORT          if we couldn't get the line
   */

#ifdef LINPLIP
   having_line = !TESTLINE(pb);
   if(!having_line)
   {
	 if (!(pb->pb_Flags & PLIPF_RECEIVING))
	    CLEARREQUEST(pb);                         /* reset line state */
	 d2(("couldn't get the line\n"));
   }
#else
   having_line = FALSE;

   if (!TESTLINE(pb))                               /* is the line free ? */
   {
      SETREQUEST(pb);                     /* indicate our request to send */
      
#if 0
   /*
   ** I have removed again the ARBITRATIONDELAY feature, although I am not
   ** really sure if this is a good thing. Anyway I didn't experience
   ** any more those nasty line errors, that initially made me implementing
   ** this. For now I've left the code here to let you play with it. Please
   ** report any comments concerning this.
   **
   ** In fact the arbitration leaves a small door for undetected, real
   ** collisision, as the request lines are used for handshake during the
   ** transmission process. The CLEARREQUEST after NOT getting the line
   ** could be misinterpreted by the other side as the first handshake. Up
   ** to now I couldn't conceive a satisfying solution for this.
   */

      if (pb->pb_ArbitrationDelay > 0)
      {
	 pb->pb_CollReq.tr_time.tv_secs    = 0;
	 pb->pb_CollReq.tr_time.tv_micro   = pb->pb_ArbitrationDelay;
	 pb->pb_CollReq.tr_node.io_Command = TR_ADDREQUEST;
	 DoIO((struct IORequest*)&pb->pb_CollReq);
      }
#endif

      if (!TESTLINE(pb))                      /* is the line still free ? */
	 having_line = TRUE;
      else
      {
	 if (!(pb->pb_Flags & PLIPF_RECEIVING))
	    CLEARREQUEST(pb);                         /* reset line state */
	 d2(("couldn't get the line-1\n"));
      }
   }
   else d2(("couldn't get the line-2\n"));
#endif /* LINPLIP */

   if (having_line)
   {
      struct BufferManagement *bm;

      if (!(pb->pb_Flags & PLIPF_RECEIVING))
      {
	 d(("having line for: type %08lx, size %ld\n",ios2->ios2_PacketType,
						      ios2->ios2_DataLength));

	 frame->pf_Type = ios2->ios2_PacketType;
	 frame->pf_Size = ios2->ios2_DataLength + PKTFRAMESIZE_2;

	 bm = (struct BufferManagement *)ios2->ios2_BufferManagement;

	 if (!(*bm->bm_CopyFromBuffer)((UBYTE*)(frame+1),
				     ios2->ios2_Data, ios2->ios2_DataLength))
	 {
	    rc = AW_BUFFER_ERROR;
	    CLEARREQUEST(pb);                         /* reset line state */
	 }
	 else
	 {
	    /* wait until I/O block is safe to be reused */
	    while(!pb->pb_TimeoutSet) Delay(1L);
	    pb->pb_TimeoutReq.tr_time.tv_secs = 0;
	    pb->pb_TimeoutReq.tr_time.tv_micro = pb->pb_Timeout;
	    pb->pb_TimeoutSet = 0;
	    SendIO((struct IORequest*)&pb->pb_TimeoutReq);
	    rc = hwsend(pb) ? AW_OK : AW_ERROR;
	    AbortIO((struct IORequest*)&pb->pb_TimeoutReq);
#if DEBUG&8
	    if(rc==AW_ERROR) d8(("Error sending packet (size=%ld)\n", (LONG)pb->pb_Frame->pf_Size));
#endif
	 }
      }
      else
      {
	 d4(("arbitration error!\n"));
	 rc = AW_ABORTED;
      }
   }
   else
      rc = AW_ABORTED;

   return rc;
}
/*E*/
/*F*/ PRIVATE REGARGS VOID dowritereqs(BASEPTR)
{
   struct IOSana2Req *currentwrite, *nextwrite;
   AW_RESULT code;

   ObtainSemaphore(&pb->pb_WriteListSem);

   for(currentwrite = (struct IOSana2Req *)pb->pb_WriteList.lh_Head;
       nextwrite = (struct IOSana2Req *) currentwrite->ios2_Req.io_Message.mn_Node.ln_Succ;
       currentwrite = nextwrite )
   {
      if (pb->pb_Flags & PLIPF_RECEIVING)
      {
	 d(("incoming data!"));
	 break;
      }

      code = arbitratedwrite(pb, currentwrite);

      if (code == AW_ABORTED)                         /* arbitration failed */
      {
	 pb->pb_Flags |= PLIPF_COLLISION;
	 d(("couldn't get the line, trying again later\n"));
	 pb->pb_SpecialStats[S2SS_COLLISIONS].Count++;
	 d(("pb->pb_SpecialStats[S2SS_COLLISIONS].Count = %ld\n",pb->pb_SpecialStats[S2SS_COLLISIONS].Count));
	 if ((currentwrite->ios2_Req.io_Error++) > pb->pb_Retries)
	 {
	    pb->pb_SpecialStats[S2SS_TXERRORS].Count++;
	    d(("pb->pb_SpecialStats[S2SS_TXERRORS].Count = %ld\n",pb->pb_SpecialStats[S2SS_TXERRORS].Count));
	    currentwrite->ios2_Req.io_Error = S2ERR_TX_FAILURE;
	    currentwrite->ios2_WireError = S2WERR_TOO_MANY_RETIRES;
	    Remove((struct Node*)currentwrite);
	    DevTermIO(pb, currentwrite);
	 }
	 break;
      }
      else if (code == AW_BUFFER_ERROR)  /* BufferManagement callback error */
      {
	 d(("buffer error\n"));
	 DoEvent(pb, S2EVENT_ERROR | S2EVENT_BUFF | S2EVENT_SOFTWARE);
	 pb->pb_SpecialStats[S2SS_TXERRORS].Count++;
	 d(("pb->pb_SpecialStats[S2SS_TXERRORS].Count = %ld\n",pb->pb_SpecialStats[S2SS_TXERRORS].Count));
	 currentwrite->ios2_Req.io_Error = S2ERR_SOFTWARE;
	 currentwrite->ios2_WireError = S2WERR_BUFF_ERROR;
	 Remove((struct Node*)currentwrite);
	 DevTermIO(pb, currentwrite);
      }
      else if (code == AW_ERROR)
      {
	 /*
	 ** this is a real line error, upper levels (e.g. Internet TCP) have
	 ** to care for reliability!
	 */
	 d(("error while transmitting packet\n"));
	 DoEvent(pb, S2EVENT_ERROR | S2EVENT_TX | S2EVENT_HARDWARE);
	 pb->pb_SpecialStats[S2SS_TXERRORS].Count++;
	 d(("pb->pb_SpecialStats[S2SS_TXERRORS].Count = %ld\n",pb->pb_SpecialStats[S2SS_TXERRORS].Count));
	 currentwrite->ios2_Req.io_Error = S2ERR_TX_FAILURE;
	 currentwrite->ios2_WireError = S2WERR_GENERIC_ERROR;
	 Remove((struct Node*)currentwrite);
	 DevTermIO(pb, currentwrite);
      }
      else /*if (code == AW_OK)*/                             /* well done! */
      {
	 d(("packet transmitted successfully\n"));
	 pb->pb_DevStats.PacketsSent++;
	 dotracktype(pb, (ULONG) pb->pb_Frame->pf_Type, 1, 0, currentwrite->ios2_DataLength, 0, 0);
	 currentwrite->ios2_Req.io_Error = S2ERR_NO_ERROR;
	 currentwrite->ios2_WireError = S2WERR_GENERIC_ERROR;
	 Remove((struct Node*)currentwrite);
	 DevTermIO(pb, currentwrite);
      }
   }

   ReleaseSemaphore(&pb->pb_WriteListSem);
}
/*E*/

   /*
   ** reading packets
   */
/*F*/ PRIVATE REGARGS VOID doreadreqs(BASEPTR)
{
   LONG datasize;
   struct IOSana2Req *got;
   ULONG pkttyp;
   struct BufferManagement *bm;
   BOOL rv;
   struct PLIPFrame *frame = pb->pb_Frame;

   /* wait until I/O block is safe to be reused */
   while(!pb->pb_TimeoutSet) Delay(1L);
   pb->pb_TimeoutReq.tr_time.tv_secs    = 0;
   pb->pb_TimeoutReq.tr_time.tv_micro   = pb->pb_Timeout;
   pb->pb_TimeoutSet = 0;
   SendIO((struct IORequest*)&pb->pb_TimeoutReq);
   rv = hwrecv(pb);
   AbortIO((struct IORequest*)&pb->pb_TimeoutReq);

   if (rv)
   {
      pb->pb_DevStats.PacketsReceived++;

      datasize = frame->pf_Size - PKTFRAMESIZE_2;

      dotracktype(pb, pkttyp = frame->pf_Type, 0, 1, 0, datasize, 0);

      d(("packet %08lx, size %ld received\n",pkttyp,datasize));

      ObtainSemaphore(&pb->pb_ReadListSem);

	 /* traverse the list of read-requests */
      for(got = (struct IOSana2Req *)pb->pb_ReadList.lh_Head;
	  got->ios2_Req.io_Message.mn_Node.ln_Succ;
	  got = (struct IOSana2Req *)got->ios2_Req.io_Message.mn_Node.ln_Succ )
      {
	    /* check if this one requests for the new packet we got */
	 if (got->ios2_PacketType == pkttyp )
	 {
	    Remove((struct Node*)got);

	    bm = (struct BufferManagement *)got->ios2_BufferManagement;

	    if (!(*bm->bm_CopyToBuffer)(got->ios2_Data, (UBYTE*)(frame+1), datasize))
	    {
	       d(("CopyToBuffer: error\n"));
	       got->ios2_Req.io_Error = S2ERR_SOFTWARE;
	       got->ios2_WireError = S2WERR_BUFF_ERROR;
	       DoEvent(pb, S2EVENT_ERROR | S2EVENT_BUFF | S2EVENT_SOFTWARE);
	    }
	    else
	    {
	       got->ios2_Req.io_Error = got->ios2_WireError = 0;
	    }

	    got->ios2_Req.io_Flags = 0;
#ifndef LINPLIP
	    memcpy(got->ios2_SrcAddr, pb->pb_SrcAddr, PLIP_ADDRFIELDSIZE);
	    memcpy(got->ios2_DstAddr, pb->pb_DstAddr, PLIP_ADDRFIELDSIZE);
#endif
	    got->ios2_DataLength = datasize;

	    d(("packet received, satisfying S2Request\n"));
	    DevTermIO(pb, got);
	    got = NULL;
	    break;
	 }
      }

      ReleaseSemaphore(&pb->pb_ReadListSem);
   }
   else
   {
      d8(("Error receiving (%ld. len=%ld)\n", rv, frame->pf_Size));
      /* something went wrong during receipt */
      DoEvent(pb, S2EVENT_HARDWARE | S2EVENT_ERROR | S2EVENT_RX);
      got = NULL;
      pb->pb_DevStats.BadData++;
   }

      /* If no one wanted this packet explicitely, there is one chance
      ** left: somebody waiting for orphaned packets. If this fails, too,
      ** we will drop it.
      */
   if (got)
   {
      d(("unknown packet\n"));

      pb->pb_DevStats.UnknownTypesReceived++;
      
      ObtainSemaphore(&pb->pb_ReadOrphanListSem);
      got = (struct IOSana2Req *)RemHead((struct List*)&pb->pb_ReadOrphanList);
      ReleaseSemaphore(&pb->pb_ReadOrphanListSem);

      if (got)
      {
	 bm = (struct BufferManagement *)got->ios2_BufferManagement;
	 if (!(*bm->bm_CopyToBuffer)(got->ios2_Data, (UBYTE*)(frame+1), datasize))
	 {
	    got->ios2_Req.io_Error = S2ERR_SOFTWARE;
	    got->ios2_WireError = S2WERR_BUFF_ERROR;
	 }
	 else
	 {
	    got->ios2_Req.io_Error = got->ios2_WireError = 0;
	 }
	 
	 got->ios2_Req.io_Flags = 0;
#ifndef LINPLIP
	 memcpy(got->ios2_SrcAddr, pb->pb_SrcAddr, PLIP_ADDRFIELDSIZE);
	 memcpy(got->ios2_DstAddr, pb->pb_DstAddr, PLIP_ADDRFIELDSIZE);
#endif
	 got->ios2_DataLength = datasize;

	 d(("orphan read\n"));

	 DevTermIO(pb, got);
      }
      else
      {
	 dotracktype(pb, pkttyp, 0, 0, 0, 0, 1);
	 d(("packet thrown away...\n"));
      }
   }
}
/*E*/

   /*
   ** 2nd level device command dispatcher (~SANA2IOF_QUICK)
   */
/*F*/ PRIVATE REGARGS VOID dos2reqs(BASEPTR)
{
   struct IOSana2Req *ios2;

   /*
   ** Every pending IO message will be GetMsg()'ed and processed. At the
   ** end of the loop it will be DevTermIO()'ed back to the sender,
   ** _but_only_if_ it is non-NULL. In such cases the message has been
   ** put in a separate queue to be DevTermIO()'ed later (i.e. CMD_WRITEs
   ** and similar stuff).
   ** You find the same mimique in the 1st level dispatcher (device.c)
   */
   while(ios2 = (struct IOSana2Req *)GetMsg(pb->pb_ServerPort))
   {
      if (pb->pb_Flags & PLIPF_RECEIVING)
      {
	 d(("incoming data!"));
	 break;
      }

      d(("sana2req %ld from serverport\n", ios2->ios2_Req.io_Command));

      switch (ios2->ios2_Req.io_Command)
      {
	 case S2_ONLINE:
	    if (!goonline(pb))
	    {
	       ios2->ios2_Req.io_Error = S2ERR_NO_RESOURCES;
	       ios2->ios2_WireError = S2WERR_GENERIC_ERROR;
	    }
	 break;

	 case S2_OFFLINE:
	    gooffline(pb);
	    rejectpackets(pb); /* reject all pending requests */
	 break;

	 case S2_CONFIGINTERFACE:
	    if (pb->pb_Flags & PLIPF_NOTCONFIGURED)
	    {
#ifndef LINPLIP
	       memcpy(ios2->ios2_SrcAddr, pb->pb_SrcAddr, PLIP_ADDRFIELDSIZE);
	       memcpy(ios2->ios2_DstAddr, pb->pb_DstAddr, PLIP_ADDRFIELDSIZE);
#endif
	       if (!goonline(pb))
	       {
		  ios2->ios2_Req.io_Error = S2ERR_NO_RESOURCES;
		  ios2->ios2_WireError = S2WERR_GENERIC_ERROR;
	       }
	    }
	    else
	    {
	       ios2->ios2_Req.io_Error = S2ERR_BAD_STATE;
	       ios2->ios2_WireError = S2WERR_IS_CONFIGURED;
	    }
	 break;
      }

      if (ios2) DevTermIO(pb,ios2);
   }
}
/*E*/

   /*
   ** startup,initialisation and termination functions
   */
/*F*/ PRIVATE struct PLIPBase *startup(void)
{
   struct ServerStartup *ss;
   struct Process *we;
   struct PLIPBase *base;
   LOCALSYSBASE;

   we = (struct Process*)FindTask(NULL);

   d(("waiting for startup msg...\n"));
   WaitPort(&we->pr_MsgPort);
   ss = (struct ServerStartup *)GetMsg(&we->pr_MsgPort);
   base = ss->ss_PLIPBase;
   base->pb_Startup = ss;
   d(("go startup msg at %lx, PLIPBase is %lx\n", ss, ss->ss_PLIPBase));

   /* we will keep the startup message, to inform mother if we
   ** really could come up or if we failed to obtain some
   ** resource.
   */
   return base;
}
/*E*/
/*F*/ PRIVATE VOID readargs(BASEPTR)
{
   struct RDArgs *rda;
   struct PLIPConfig args = { 0 };
   BPTR plipvar, oldinput;

   d(("entered\n"));

   if (plipvar = Open(CONFIGFILE, MODE_OLDFILE))
   {
      oldinput = SelectInput(plipvar);
      
      rda = ReadArgs(TEMPLATE , (LONG *)&args, NULL);
      
      if(rda)
      {
	 if (args.timeout)
	    pb->pb_Timeout =
		  BOUNDS(*args.timeout, PLIP_MINTIMEOUT, PLIP_MAXTIMEOUT);

	 if (args.priority)
	    SetTaskPri((struct Task*)pb->pb_Server,
		  BOUNDS(*args.priority, PLIP_MINPRIORITY, PLIP_MAXPRIORITY));

	 if (args.mtu)
	    pb->pb_MTU = BOUNDS(*args.mtu, PLIP_MINMTU, PLIP_MAXMTU);

	 if (args.bps)
	    pb->pb_ReportBPS = BOUNDS(*args.bps, PLIP_MINBPS, PLIP_MAXBPS);

	 if (args.retries)
	    pb->pb_Retries =
		     BOUNDS(*args.retries, PLIP_MINRETRIES, PLIP_MAXRETRIES);

	 if (args.sendcrc)
	    pb->pb_Flags |= PLIPF_SENDCRC;
	  else
	    pb->pb_Flags &= ~PLIPF_SENDCRC;

	 if (args.collisiondelay)
	    pb->pb_CollisionDelay =
	       BOUNDS(*args.collisiondelay, PLIP_MINCOLLISIONDELAY,
					    PLIP_MAXCOLLISIONDELAY);
	 else
	    pb->pb_CollisionDelay = PLIP_DEFDELAY + (pb->pb_Unit ?
						  PLIP_DELAYDIFF : 0);

	 if (args.arbitrationdelay)
	    pb->pb_ArbitrationDelay =
	       BOUNDS(*args.collisiondelay, PLIP_MINARBITRATIONDELAY,
					    PLIP_MAXARBITRATIONDELAY);
	 else
	    pb->pb_ArbitrationDelay = PLIP_DEFARBITRATIONDELAY;

	 if (args.nospecialstats)
	    pb->pb_ExtFlags |= PLIPEF_NOSPECIALSTATS;

	 FreeArgs(rda);
      }

      Close(SelectInput(oldinput));
   }

   d(("timeout %ld, pri %ld, mtu %ld, bps %ld, retries %ld, flags %08lx, delay %ld\n",
      pb->pb_Timeout, (LONG)pb->pb_Server->pr_Task.tc_Node.ln_Pri, pb->pb_MTU, pb->pb_ReportBPS, pb->pb_Retries,
      pb->pb_Flags, pb->pb_CollisionDelay));

   d(("left\n"));

}
/*E*/
/*F*/ PRIVATE BOOL init(BASEPTR)
{
   BOOL rc = FALSE;
   ULONG sigmask;

   if ((pb->pb_IntSig = AllocSignal(-1)) != -1)
   {
      pb->pb_IntSigMask = 1L << pb->pb_IntSig;
   
      if ((pb->pb_ServerPort = CreateMsgPort()))
      {
	 if ((pb->pb_CollPort = CreateMsgPort()))
	 {
	    if ((pb->pb_TimeoutPort = CreateMsgPort()))
	    {
	       /* save old exception setup */
	       pb->pb_OldExcept = SetExcept(0, 0xffffffff); /* turn'em off */
	       pb->pb_OldExceptCode = pb->pb_Server->pr_Task.tc_ExceptCode;
	       pb->pb_OldExceptData = pb->pb_Server->pr_Task.tc_ExceptData;

	       /* create new exception setup */
	       pb->pb_Server->pr_Task.tc_ExceptCode = (APTR)&exceptcode;
	       pb->pb_Server->pr_Task.tc_ExceptData = (APTR)pb;
	       SetSignal(0, sigmask = (1 << pb->pb_TimeoutPort->mp_SigBit));
	       SetExcept(sigmask, sigmask);

	       /* enter port address */
	       pb->pb_CollReq.tr_node.io_Message.mn_ReplyPort = pb->pb_CollPort;
	       if (!OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest*)&pb->pb_CollReq, 0))
	       {
		  TimerBase = (struct Library *)pb->pb_CollReq.tr_node.io_Device;

		  /* preset the io command, this will never change */
		  pb->pb_CollReq.tr_node.io_Command = TR_ADDREQUEST;

		  /* setup the timeout stuff */
		  pb->pb_TimeoutReq.tr_node.io_Flags = IOF_QUICK;
		  pb->pb_TimeoutReq.tr_node.io_Message.mn_ReplyPort = pb->pb_TimeoutPort;
		  pb->pb_TimeoutReq.tr_node.io_Device = pb->pb_CollReq.tr_node.io_Device;
		  pb->pb_TimeoutReq.tr_node.io_Unit = pb->pb_CollReq.tr_node.io_Unit;
		  pb->pb_TimeoutReq.tr_node.io_Command = TR_ADDREQUEST;
		  pb->pb_TimeoutSet = 0xff;

		  readargs(pb);
		  d(("allocating 0x%lx/%ld bytes frame buffer\n",
				       sizeof(struct PLIPFrame)+pb->pb_MTU,
				       sizeof(struct PLIPFrame)+pb->pb_MTU));
		  if ((pb->pb_Frame = AllocVec((ULONG)sizeof(struct PLIPFrame) +
						  pb->pb_MTU, MEMF_CLEAR|MEMF_ANY)))
		  {
		     rc = TRUE;
		  }
		  else
		  {
		     d(("couldn't allocate frame buffer\n"));
		  }
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
	    d(("no port for collision handling\n"));
	 }
      }
      else
      {
	 d(("no server port\n"));
      }
   }
   else
   {
      d(("no signal\n",rc));
   }

   d(("left %ld\n",rc));

   return rc;
}
/*E*/
/*F*/ PRIVATE VOID cleanup(BASEPTR)
{
   struct BufferManagement *bm;

   gooffline(pb);

   while(bm = (struct BufferManagement *)RemHead((struct List *)&pb->pb_BufferManagement))
      FreeVec(bm);

   if (pb->pb_Frame) FreeVec(pb->pb_Frame);

   if (pb->pb_TimeoutPort)
   {
      /* restore old exception setup */
      SetExcept(0, 0xffffffff);    /* turn'em off */
      pb->pb_Server->pr_Task.tc_ExceptCode = pb->pb_OldExceptCode;
      pb->pb_Server->pr_Task.tc_ExceptData = pb->pb_OldExceptData;
      SetExcept(pb->pb_OldExcept, 0xffffffff);

      if (TimerBase)
      {
	 WaitIO((struct IORequest*)&pb->pb_TimeoutReq);
	 CloseDevice((struct IORequest*)&pb->pb_CollReq);
      }
      DeleteMsgPort(pb->pb_TimeoutPort);
   }
   if (pb->pb_CollPort) DeleteMsgPort(pb->pb_CollPort);

   if (pb->pb_ServerPort) DeleteMsgPort(pb->pb_ServerPort);
   if (pb->pb_IntSig != -1) FreeSignal(pb->pb_IntSig);

   if (pb->pb_Flags & PLIPF_REPLYSS)
   {
      Forbid();
      ReplyMsg((struct Message*)pb->pb_Startup);
   }
}
/*E*/

   /*
   ** entry point, mainloop
   */
/*F*/ PUBLIC VOID SAVEDS ServerTask(void)
{
   BASEPTR;

   d(("server running\n"));

   if (pb = startup())
   {
	 /* if we fail to allocate all resources, this flag reminds cleanup()
	 ** to ReplyMsg() the startup message
	 */
      pb->pb_Flags |= PLIPF_REPLYSS;

      if (init(pb))
      {
	 ULONG recv=0, portsigmask, timersigmask, wmask;
	 BOOL running, timerqueued = FALSE;

	 /* Ok, we are fine and will tell this mother personally :-) */
	 pb->pb_Startup->ss_Error = 0;
	 /* don't forget this, or we will have to keep a warm place */
	 /* in our coffin for the system */
	 pb->pb_Flags &= ~PLIPF_REPLYSS;
	 ReplyMsg((struct Message*)pb->pb_Startup);

	 portsigmask  = 1 << pb->pb_ServerPort->mp_SigBit;
	 timersigmask = 1 << pb->pb_CollPort->mp_SigBit;
      
	 wmask = SIGBREAKF_CTRL_F | SIGBREAKF_CTRL_C | pb->pb_IntSigMask | portsigmask | timersigmask;

	 for(running=TRUE;running;)
	 {
	    d(("wmask is 0x%08lx\n", wmask));

	    if (!(pb->pb_Flags & PLIPF_RECEIVING))
	       recv = Wait(wmask);
	    else
	       SetSignal(0, pb->pb_IntSigMask);

	    /*if (recv & pb->pb_IntSigMask)*/
	    if (pb->pb_Flags & PLIPF_RECEIVING)
	    {
	       d(("received an interrupt\n"));
	       doreadreqs(pb);
	    }

	    if (recv & portsigmask)
	    {
	       d(("SANA-II request(s)\n"));
	       dos2reqs(pb);
	    }

	    if (recv & timersigmask)
	    {
	       /* pop message */
	       AbortIO((struct IORequest*)&pb->pb_CollReq);
	       WaitIO((struct IORequest*)&pb->pb_CollReq);
	       timerqueued = FALSE;
	       d(("timer wakeup\n"));
	    }

	       /* try now to do write requests (if any pending) */
	    if (!timerqueued)
	    {
	       dowritereqs(pb);

		  /* don't let the other side wait too long! */
	       if (pb->pb_Flags & PLIPF_RECEIVING)
	       {
		  d(("received an interrupt\n"));
		  SetSignal(0, pb->pb_IntSigMask);
		  doreadreqs(pb);
	       }

	       /*
	       ** Possible a collision has occurred, which is indicated by a
	       ** special flag in PLIPBase.
	       **
	       ** Using timer.device we periodically will be waked up. This
	       ** allows us to delay write packets in cases when we cannot get
	       ** the line immediately.
	       **
	       ** If client and server are very close together, regarding the point
	       ** of performance, the same delay time could even force multiple
	       ** collisions (at least theoretical, I made no practical tests).
	       ** Probably a CSMA/CD-like random-timed delay would be ideal.
	       */
	       if (pb->pb_Flags & PLIPF_COLLISION)
	       {
		  pb->pb_Flags &= ~PLIPF_COLLISION;
		  pb->pb_CollReq.tr_time.tv_secs    = 0;
		  pb->pb_CollReq.tr_time.tv_micro   = pb->pb_CollisionDelay;
		  SendIO((struct IORequest*)&pb->pb_CollReq);
		  timerqueued = TRUE;
	       }
	    }

	    if (recv & SIGBREAKF_CTRL_C)
	    {
	       d(("received break signal\n"));
	       running = FALSE;
	    }
	 }

	 if (timerqueued)
	 {
	       /* finnish pending timer requests */
	    AbortIO((struct IORequest*)&pb->pb_CollReq);
	    WaitIO((struct IORequest*)&pb->pb_CollReq);
	 }
      }
      else
	 d(("init() failed\n"));

      d(("cleaning up\n"));
      cleanup(pb);

	    /* Exec will enable it's scheduler after we're dead. */
      Forbid();
	    /* signal mother we're done */
      if (pb->pb_ServerStoppedSigMask)
	 Signal(pb->pb_Task, pb->pb_ServerStoppedSigMask);
      pb->pb_Flags |= PLIPF_SERVERSTOPPED;
   }
   else
      d(("no startup packet\n"));
}
/*E*/

