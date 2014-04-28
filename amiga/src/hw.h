/* hw.h - hardware module of plihwbox.device */
#ifndef __HW_H
#define __HW_H

/* ----- hardware frame structure ----- */

   /* transport ethernet addresses */
#define HW_ADDRFIELDSIZE         6

#define HW_ETH_HDR_SIZE          14       /* ethernet header: dst, src, type */

struct HWFrame {
   USHORT   hwf_Size;
   /* use layout of ethernet header here */
   UBYTE    hwf_DstAddr[HW_ADDRFIELDSIZE];
   UBYTE    hwf_SrcAddr[HW_ADDRFIELDSIZE];
   USHORT   hwf_Type;
   /*UBYTE    hwf_Data[MTU];*/
};

#include "hwbase.h"

struct PLIPBase;

GLOBAL BOOL hw_init(struct PLIPBase *pb);
GLOBAL VOID hw_cleanup(struct PLIPBase *pb);

GLOBAL BOOL hw_attach(struct PLIPBase *pb);
GLOBAL VOID hw_detach(struct PLIPBase *pb);

GLOBAL BOOL hw_send_frame(struct PLIPBase *pb, struct HWFrame *frame);

GLOBAL ULONG hw_recv_sigmask(struct PLIPBase *pb);
GLOBAL BOOL hw_recv_pending(struct PLIPBase *pb);
GLOBAL BOOL hw_recv_frame(struct PLIPBase *pb, struct HWFrame *frame);

#endif
