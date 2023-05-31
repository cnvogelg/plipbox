#ifndef __GLOBAL_H
#define __GLOBAL_H

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
#ifndef __HW_H
#include "hw.h"
#endif


/****************************************************************************/

   /* return val, cut to min or max if exceeding range */
#define BOUNDS(val, min, max) ((val) <= (max) ? ((val) >= (min) ? (val) :\
                                                         (min)) : (max))

   /* memory economy is *everything* :-) */
#define SERVERTASKNAME           pb->pb_DevNode.lib_Node.ln_Name

struct TrackRec {
   struct MinNode              tr_Link;
   ULONG                       tr_PacketType;
   ULONG                       tr_Count;
   struct Sana2PacketTypeStats tr_Sana2PacketTypeStats;
};


/****************************************************************************/


typedef BOOL (* ASM BMFunc)(REG(a0,void *), REG(a1,void *), REG(d0,LONG));

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
   struct Library          *   pb_UtilityBase;
   struct Library          *   pb_DOSBase;
   struct Library          *   pb_SysBase;
   struct ServerStartup    *   pb_Startup;            /* main server proces */
   struct Process          *   pb_Server;             /* main server proces */
   struct Task             *   pb_Task;         /* used for server shutdown */
   ULONG                       pb_ServerStoppedSigMask;     /* for shutdown */
   struct MsgPort          *   pb_ServerPort;       /* for IOReq forwarding */
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

   volatile UBYTE              pb_Flags;                       /* see below */
   UBYTE                       pb_pad2;
   volatile UWORD              pb_ExtFlags;                    /* see below */
   UBYTE                       pb_CfgAddr[HW_ADDRFIELDSIZE];
   UBYTE                       pb_DefAddr[HW_ADDRFIELDSIZE];
   struct HWBase               pb_HWBase;
   struct HWFrame        *     pb_Frame;
   ULONG                       pb_BPS;
   ULONG                       pb_MTU;
};

     /*
     ** redirect all shared library bases to our device base.
     */
#define SysBase      pb->pb_SysBase
#define DOSBase      pb->pb_DOSBase
#define UtilityBase  pb->pb_UtilityBase
     /*
     ** This macro declares a local variable which temporary gets
     ** SysBase directly from AbsExecBase.
     */
#define LOCALSYSBASE struct { void *pb_SysBase; } *pb = (void*)0x4
     /*
     ** Use this macro as argument for all functions which need to
     ** have access to your data base.
     */
#define BASEPTR      struct PLIPBase *pb

   /*
   ** Values for PLIPBase->pb_Flags
   ** Note that the Flags field is intentionally only 8 bits wide so that
   ** the ASM parts may use the bit functions.
   */
#define PLIPB_REPLYSS         0   /* server-startup must be replied */
#define PLIPB_EXCLUSIVE       1   /* current opener is exclusive */
#define PLIPB_OFFLINE         2   /* currently not online (sic!) */
#define PLIPB_SERVERSTOPPED   3   /* set by server while passing away */

#define PLIPF_REPLYSS         (1<<PLIPB_REPLYSS)
#define PLIPF_EXCLUSIVE       (1<<PLIPB_EXCLUSIVE)
#define PLIPF_OFFLINE         (1<<PLIPB_OFFLINE)
#define PLIPF_SERVERSTOPPED   (1<<PLIPB_SERVERSTOPPED)

   /*
   ** Values for PLIPBase->pb_ExtFlags
   */
#define PLIPEB_NOSPECIALSTATS 0   /* don't report special stats */
#define PLIPEF_NOSPECIALSTATS (1<<PLIPEB_NOSPECIALSTATS)

#endif
