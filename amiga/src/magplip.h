#ifndef __MAGPLIP_H
#define __MAGPLIP_H
/*
** $VER: magplip.h 1.15 (01 Apr 1998)
**
** magplip.device - Parallel Line Internet Protocol
**
** Original code written by Oliver Wagner and Michael Balzer.
**
** This version has been completely reworked by Marius Gröger,
** introducing slight protocol changes. The new source is
** a lot better organized and maintainable.
**
** Additional changes and code cleanup by Jan Kratochvil and Martin Mares.
** The new source is significantly faster and yet better maintainable.
**
** (C) Copyright 1993-1994 Oliver Wagner & Michael Balzer
** (C) Copyright 1995 Jan Kratochvil & Martin Mares
** (C) Copyright 1995 Marius Gröger
**     All Rights Reserved
**
** $HISTORY:
**
** 01 Apr 1998 : 001.015 :  intergated linPLIP modifications from Stephane
** 10 Apr 1996 : 001.014 :  + PLIPF_REPLYSS didn't fit into byte
**                          + pb_ExtFlags
** 09 Apr 1996 : 001.013 :  added TrackRe.tr_Count
** 29 Mar 1996 : 001.012 :  changed copyright note
** 30 Dec 1995 : 001.011 :  + single dynamic frame buffer
**                          + added a lot of min/max constants
** 29 Dec 1995 : 001.010 :  + pb_Startup
**                          + new flag PLIPF_REPLYSS
** 03 Sep 1995 : 001.009 :  + removed PLIP(F|B)_SIDEA
**                          + hardware addressing fields in PLIPBase
** 30 Aug 1995 : 001.008 :  support for timer-timed timeout :-)
**                          some pecularities moved to compiler.h
** 13 Aug 1995 : 001.007 :  code cleanup
** 29 Jul 1995 : 001.006 :  support for arbitration delay
** 24 Jul 1995 : 001.005 :  only one delay value in config
**                          some volatile elements in GD
** 25 Apr 1995 : 001.004 :  #define's FAR
** 06 Mar 1995 : 001.003 :  added collision delays
** 04 Mar 1995 : 001.002 :  packet-type now ULONG
** 18 Feb 1995 : 001.001 :  some SAS/C wrappers, better data base handling
** 12 Feb 1995 : 001.000 :  reworked original
*/

   /* system header files */
#ifndef DEVICES_SANA2_H
#include <devices/sana2.h>
#endif
#ifndef EXEC_SEMAPHORES_H
#include <exec/semaphores.h>
#endif
#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif
#ifndef EXEC_INTERRUPTS_H
#include <exec/interrupts.h>
#endif
#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif
#ifndef DOS_DOS_H
#include <dos/dos.h>
#endif

#ifndef __COMPILER_H
#include "compiler.h"
#endif


/****************************************************************************/


   /* memory economy is *everything* :-) */
#define SERVERTASKNAME           pb->pb_DevNode.lib_Node.ln_Name

      /* default values */
#define PLIP_DEFMTU              1500
#define PLIP_DEFBPS              100000
#define PLIP_DEFRETRIES          32
#define PLIP_DEFTIMEOUT          (500*1000)
#define PLIP_DEFDELAY            0
#define PLIP_DELAYDIFF           2000
#define PLIP_DEFARBITRATIONDELAY 500

      /* minimum values */
#define PLIP_MINPRIORITY         -128
#define PLIP_MINMTU              64
#define PLIP_MINRETRIES          1
#define PLIP_MINCOLLISIONDELAY   0
#define PLIP_MINARBITRATIONDELAY 0
#define PLIP_MINTIMEOUT          500
#define PLIP_MINBPS              1

      /* maximum values */
#define PLIP_MAXMTU              1500
#define PLIP_MAXRETRIES          127   /* don't try higher values! */
#define PLIP_MAXPRIORITY         127
#define PLIP_MAXCOLLISIONDELAY   999999
#define PLIP_MAXARBITRATIONDELAY 999999
#define PLIP_MAXTIMEOUT          999999
#define PLIP_MAXBPS              0x7fffffff

   /* transport ethernet addresses */
#define PLIP_ADDRFIELDSIZE       6

/****************************************************************************/

   /* this _is_ awful, I know */
#define PKTFRAMESIZE_1           4        /* the sync-field and packet-size */
#define PKTFRAMESIZE_2           2        /* crc only */
#define PKTFRAMESIZE_3           14       /* ethernet header: dst, src, type */

struct PLIPFrame {
   USHORT   pf_Sync;
   SHORT    pf_Size;
   USHORT   pf_CRC;
   /* use layout of ethernet header here */
   UBYTE    pf_DstAddr[PLIP_ADDRFIELDSIZE];
   UBYTE    pf_SrcAddr[PLIP_ADDRFIELDSIZE];
   USHORT   pf_Type;
   /*UBYTE    pf_Data[MTU];*/
};


#define SYNCBYTE_HEAD   0x42
#define SYNCBYTE_CRC    0x01
#define SYNCBYTE_NOCRC  0x02
#define SYNCWORD_CRC    ((SYNCBYTE_HEAD << 8) | SYNCBYTE_CRC)
#define SYNCWORD_NOCRC  ((SYNCBYTE_HEAD << 8) | SYNCBYTE_NOCRC)

/****************************************************************************/


struct TrackRec {
   struct MinNode              tr_Link;
   ULONG                       tr_PacketType;
   ULONG                       tr_Count;
   struct Sana2PacketTypeStats tr_Sana2PacketTypeStats;
};


/****************************************************************************/


typedef BOOL (* ASM BMFunc)(REG(a0) void *, REG(a1) void *, REG(d0) LONG);

struct BufferManagement
{
    struct MinNode   bm_Node;
    BMFunc           bm_CopyFromBuffer;
    BMFunc           bm_CopyToBuffer;
};


/****************************************************************************/


   /*
   ** sent to the server task after being CreateNewProc()ed
   */
struct ServerStartup
{
   struct Message    ss_Msg;
   struct PLIPBase  *ss_PLIPBase;
   BOOL              ss_Error;
   UBYTE             ss_Pad[2];
};



/****************************************************************************/


enum { S2SS_TXERRORS, S2SS_COLLISIONS, S2SS_COUNT };


   /*
   ** This count will record the number of packets that failed
   ** to be transmitted.
   */
#define S2SS_PLIP_TXERRORS ((((S2WireType_Ethernet) & 0xffff) << 16) | (S2SS_TXERRORS))

   /*
   ** This count will record the number of line arbitration collisions
   */
#define S2SS_PLIP_COLLISIONS ((((S2WireType_Ethernet) & 0xffff) << 16) | (S2SS_COLLISIONS))


/****************************************************************************/


struct PLIPBase
{
   struct Library              pb_DevNode;        /* basic device structure */
   UBYTE                       pb_Unit;         /* unit of 1st OpenDevice() */
   UBYTE                       pb_pad1; /* make the following long alligned */
   BPTR                        pb_SegList;               /* pointer to code */
   struct Library          *   pb_MiscBase,          /* various libs & res. */
                           *   pb_CIAABase,
                           *   pb_UtilityBase,
                           *   pb_TimerBase,
                           *   pb_DOSBase,
                           *   pb_SysBase;
   struct ServerStartup    *   pb_Startup;            /* main server proces */
   struct Process          *   pb_Server;             /* main server proces */
   struct Task             *   pb_Task;         /* used for server shutdown */
   struct Interrupt            pb_Interrupt;          /* for AddICRVector() */
   ULONG                       pb_IntSig;        /* sent from int to server */
   ULONG                       pb_IntSigMask;                  /* for speed */
   ULONG                       pb_ServerStoppedSigMask;     /* for shutdown */
   struct MsgPort          *   pb_ServerPort,       /* for IOReq forwarding */
                           *   pb_TimeoutPort,      /* for timeout handling */
                           *   pb_CollPort;       /* for collision handling */
   struct timerequest          pb_TimeoutReq,       /* for timeout handling */
                               pb_CollReq;        /* for collision handling */
   struct Sana2DeviceStats     pb_DevStats;            /* SANA-2 wants this */
   struct Sana2SpecialStatRecord
                               pb_SpecialStats[S2SS_COUNT];
   volatile struct List        pb_ReadList,                  /* the readers */
                               pb_WriteList,                 /* the writers */
                               pb_EventList,              /* event tracking */
                               pb_ReadOrphanList,   /* for spurious packets */
                               pb_TrackList,                  /* track type */
                               pb_BufferManagement;          /* Copy-In/Out */
   struct SignalSemaphore      pb_EventListSem,     /* protection for lists */
                               pb_ReadListSem,
                               pb_WriteListSem,
                               pb_TrackListSem,
                               pb_ReadOrphanListSem,
                               pb_Lock;
   ULONG                       pb_Retries;                 /* config values */
   ULONG                       pb_ReportBPS;
   ULONG                       pb_MTU;
   ULONG                       pb_AllocFlags;
   ULONG                       pb_Timeout;
   LONG                        pb_CollisionDelay;
   LONG                        pb_ArbitrationDelay;
   volatile UBYTE              pb_TimeoutSet;/* if != 0, a timeout occurred */
   volatile UBYTE              pb_Flags;                       /* see below */
   volatile UWORD              pb_ExtFlags;                    /* see below */
   APTR                        pb_OldExceptCode;
   APTR                        pb_OldExceptData;
   ULONG                       pb_OldExcept;
   UBYTE                       pb_CfgAddr[PLIP_ADDRFIELDSIZE];
   UBYTE                       pb_DefAddr[PLIP_ADDRFIELDSIZE];
   struct PLIPFrame        *   pb_Frame;
};

#ifdef __SASC
     /*
     ** redirect all shared library bases to our device base.
     */
#  define SysBase      pb->pb_SysBase
#  define DOSBase      pb->pb_DOSBase
#  define TimeBase     pb->pb_TimeBase
#  define UtilityBase  pb->pb_UtilityBase
#  define MiscBase     pb->pb_MiscBase
#  define CIAABase     pb->pb_CIAABase
#  define CiaBase      pb->pb_CIAABase
#  define TimerBase    pb->pb_TimerBase
     /*
     ** This macro declares a local variable which temporary gets
     ** SysBase directly from AbsExecBase.
     */
#  define LOCALSYSBASE struct { void *pb_SysBase; } *pb = (void*)0x4
     /*
     ** Use this macro as argument for all functions which need to
     ** have access to your data base.
     */
#  define BASEPTR      struct PLIPBase *pb
#else
#  error Please define library bases for your compiler
#endif

   /*
   ** Values for PLIPBase->pb_Flags
   ** Note that the Flags field is intentionally only 8 bits wide so that
   ** the ASM parts may use the bit functions.
   */
#define PLIPB_REPLYSS         0   /* server-startup must be replied */
#define PLIPB_EXCLUSIVE       1   /* current opener is exclusive */
#define PLIPB_NOTCONFIGURED   2   /* not configured */
#define PLIPB_OFFLINE         3   /* currently not online (sic!) */
#define PLIPB_SENDCRC         4   /* send with CRC sum */
#define PLIPB_RECEIVING       5   /* set by interrupt */
#define PLIPB_COLLISION       6   /* arbitration detected a collision */
#define PLIPB_SERVERSTOPPED   7   /* set by server while passing away */

#define PLIPF_EXCLUSIVE       (1<<PLIPB_EXCLUSIVE)
#define PLIPF_NOTCONFIGURED   (1<<PLIPB_NOTCONFIGURED)
#define PLIPF_OFFLINE         (1<<PLIPB_OFFLINE)
#define PLIPF_SENDCRC         (1<<PLIPB_SENDCRC)
#define PLIPF_RECEIVING       (1<<PLIPB_RECEIVING)
#define PLIPF_COLLISION       (1<<PLIPB_COLLISION)
#define PLIPF_SERVERSTOPPED   (1<<PLIPB_SERVERSTOPPED)
#define PLIPF_REPLYSS         (1<<PLIPB_REPLYSS)

   /*
   ** Values for PLIPBase->pb_ExtFlags
   */
#define PLIPEB_NOSPECIALSTATS 0   /* don't report special stats */
#define PLIPEF_NOSPECIALSTATS (1<<PLIPEB_NOSPECIALSTATS)


/****************************************************************************/


   /*
   ** configuration stuff
   */
#define CONFIGFILE "ENV:SANA2/plipbox.config"

#define TEMPLATE "TIMEOUT/K/N,PRIORITY=PRI/K/N,MTU/K/N,BPS/K/N,RETRIES/K/N,SENDCRC/S,CD=COLLISIONDELAY/K/N,AD=ARBITRATIONDELAY/K/N,NOSPECIALSTATS/S"

struct PLIPConfig
{
      ULONG *timeout;
      LONG  *priority;
      LONG  *mtu;
      LONG  *bps;
      LONG  *retries;
      ULONG  sendcrc;
      LONG  *collisiondelay;
      LONG  *arbitrationdelay;
      ULONG  nospecialstats;
};

#endif
