/* hw.h - hardware module of plihwbox.device */
#ifndef HW_H
#define HW_H

/* ----- hardware frame structure ----- */

/* transport ethernet addresses */
#define HW_ADDRFIELDSIZE         6

#define HW_ETH_HDR_SIZE          14       /* ethernet header: dst, src, type */
#define HW_ETH_MTU               1500

/* reported BPS (bits! per second) for this device */
#define HW_BPS (60 * 1024 * 8) /* 50 KiB/s */

struct HWFrame {
   USHORT   hwf_Size;
   /* use layout of ethernet header here */
   UBYTE    hwf_DstAddr[HW_ADDRFIELDSIZE];
   UBYTE    hwf_SrcAddr[HW_ADDRFIELDSIZE];
   USHORT   hwf_Type;
   /*UBYTE    hwf_Data[MTU];*/
};

/* ----- config stuff ----- */

struct PLIPBase;

/* hw API */
GLOBAL REGARGS BOOL hw_init(struct PLIPBase *pb);
GLOBAL REGARGS VOID hw_cleanup(struct PLIPBase *pb);

GLOBAL REGARGS BOOL hw_attach(struct PLIPBase *pb);
GLOBAL REGARGS VOID hw_detach(struct PLIPBase *pb);

GLOBAL REGARGS void hw_get_sys_time(struct PLIPBase *pb, struct timeval *time);

GLOBAL REGARGS BOOL hw_send_frame(struct PLIPBase *pb, struct HWFrame *frame);

GLOBAL REGARGS ULONG hw_recv_sigmask(struct PLIPBase *pb);
GLOBAL REGARGS BOOL hw_recv_pending(struct PLIPBase *pb);
GLOBAL REGARGS BOOL hw_recv_frame(struct PLIPBase *pb, struct HWFrame *frame);

GLOBAL REGARGS void hw_config_init(struct PLIPBase *pb,
                                   STRPTR *template_str,
                                   struct CommonConfig **cfg,
                                   STRPTR *config_file);
GLOBAL REGARGS void hw_config_update(struct PLIPBase *pb);
GLOBAL REGARGS void hw_config_dump(struct PLIPBase *pb);

#endif
