/* hwbase.h - hardware specific part of driver structure */
#ifndef __HWBASE_H
#define __HWBASE_H

/* ----- base structure for hardware ----- */

struct HWBase
{
   /* used in asm module */
   struct Library          *   hwb_SysBase;
   struct Library          *   hwb_CIAABase;
   struct Process          *   hwb_Server;
   ULONG                       hwb_IntSigMask;
   UWORD                       hwb_MaxMTU;
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
};

#define HWB_RECV_PENDING           0
#define HWB_COLL_TIMER_RUNNING     1

#define HWF_RECV_PENDING           (1 << HWB_RECV_PENDING)

/* transparently map proto lib bases to structure */
#define MiscBase     hwb->hwb_MiscBase
#define CIAABase     hwb->hwb_CIAABase
#define CiaBase      hwb->hwb_CIAABase
#define TimerBase    hwb->hwb_TimerBase

#endif
