/* hwbase.h - hardware specific part of driver structure */
#ifndef __HWBASE_H
#define __HWBASE_H

/* ----- base structure for hardware ----- */

/* reported BPS (bits! per second) for this device */
#define HW_BPS (60 * 1024 * 8) /* 50 KiB/s */

struct HWBase
{
   /* used in asm module */
   struct Library          *   hwb_SysBase;
   struct Library          *   hwb_CIAABase;
   struct Process          *   hwb_Server;
   ULONG                       hwb_IntSigMask;
   UWORD                       hwb_MaxFrameSize;
   volatile UBYTE              hwb_TimeoutSet;/* if != 0, a timeout occurred */
   volatile UBYTE              hwb_Flags;
   /* NOT used in asm */
   ULONG                       hwb_IntSig;        /* sent from int to server */
   ULONG                       hwb_CollSigMask;
   struct MsgPort          *   hwb_TimeoutPort;      /* for timeout handling */
   struct Library          *   hwb_TimerBase;
   struct Library          *   hwb_MiscBase;          /* various libs & res. */
   APTR                        hwb_OldExceptCode;
   APTR                        hwb_OldExceptData;
   ULONG                       hwb_OldExcept;
   struct Interrupt            hwb_Interrupt;          /* for AddICRVector() */
   struct timerequest          hwb_TimeoutReq,       /* for timeout handling */
                               hwb_CollReq;        /* for collision handling */
   ULONG                       hwb_AllocFlags;

   /* config options */
   ULONG                       hwb_TimeOutMicros;
   ULONG                       hwb_TimeOutSecs;
   UWORD                       hwb_BurstSize;    /* in words -1 */
};

#define HWB_RECV_PENDING           0
#define HWB_COLL_TIMER_RUNNING     1

#define HWF_RECV_PENDING           (1 << HWB_RECV_PENDING)

/* transparently map proto lib bases to structure */
#define MiscBase     hwb->hwb_MiscBase
#define CIAABase     hwb->hwb_CIAABase
#define CiaBase      hwb->hwb_CIAABase
#define TimerBase    hwb->hwb_TimerBase

/* ----- config ----- */

#define CONFIGFILE "ENV:SANA2/plipbox.config"
#define TEMPLATE "TIMEOUT/K/N,BURST/K/N"

/* structure to be filled by ReadArgs template */ 
struct TemplateConfig
{
   struct CommonConfig common;
   ULONG *timeout;
   ULONG *burst;
};

#endif
