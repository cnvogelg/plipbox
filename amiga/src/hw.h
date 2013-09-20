/* hw.h - hardware module of plihwbox.device */
#ifndef __HW_H
#define __HW_H

/* ----- hardware frame structure ----- */

   /* transport ethernet addresses */
#define HW_ADDRFIELDSIZE         6

#define HW_EXTRA_HDR_SIZE        2        /* extra fields accounted
                                             for in hw frame size */
#define HW_ETH_HDR_SIZE          14       /* ethernet header: dst, src, type */

struct HWFrame {
   USHORT   hwf_Sync;
   SHORT    hwf_Size;
   /* extra fields accounted in hwf_Size */
   USHORT   hwf_CRC;
   /* use layout of ethernet header here */
   UBYTE    hwf_DstAddr[HW_ADDRFIELDSIZE];
   UBYTE    hwf_SrcAddr[HW_ADDRFIELDSIZE];
   USHORT   hwf_Type;
   /*UBYTE    hwf_Data[MTU];*/
};

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
   struct MsgPort          *   hwb_CollPort;       /* for collision handling */
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
#define HWB_SEND_CRC               2

#define HWF_RECV_PENDING           (1 << HWB_RECV_PENDING)
#define HWF_COLL_TIMER_RUNNING     (1 << HWB_COLL_TIMER_RUNNING)
#define HWF_SEND_CRC               (1 << HWB_SEND_CRC)

#define MiscBase     hwb->hwb_MiscBase
#define CIAABase     hwb->hwb_CIAABase
#define CiaBase      hwb->hwb_CIAABase
#define TimerBase    hwb->hwb_TimerBase

struct PLIPBase;

GLOBAL BOOL hw_init(struct PLIPBase *pb);
GLOBAL VOID hw_cleanup(struct PLIPBase *pb);

GLOBAL ULONG hw_get_sigmask(struct PLIPBase *pb);
GLOBAL BOOL hw_handle_sigmask(struct PLIPBase *pb, ULONG sigmask);

GLOBAL BOOL hw_attach(struct PLIPBase *pb);
GLOBAL VOID hw_detach(struct PLIPBase *pb);

GLOBAL BOOL hw_can_send(struct PLIPBase *pb);
GLOBAL BOOL hw_begin_send(struct PLIPBase *pb);
GLOBAL VOID hw_abort_send(struct PLIPBase *pb);
GLOBAL BOOL hw_send_frame(struct PLIPBase *pb, struct HWFrame *frame);
GLOBAL VOID hw_handle_collision(struct PLIPBase *pb);

GLOBAL BOOL hw_recv_pending(struct PLIPBase *pb);
GLOBAL VOID hw_recv_ack(struct PLIPBase *pb);
GLOBAL BOOL hw_recv_frame(struct PLIPBase *pb, struct HWFrame *frame);

#endif
