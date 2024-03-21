/* hw.h - hardware module of plihwbox.device */
#ifndef HW_H
#define HW_H

/* ----- hardware frame structure ----- */

/* transport ethernet addresses */
#define HW_ADDRFIELDSIZE 6

#define HW_ETH_HDR_SIZE 14 /* ethernet header: dst, src, type */
#define HW_ETH_MTU 1500
#define HW_ETH_FRAME_SIZE 1514

/* reported BPS (bits! per second) for this device */
#define HW_BPS (60 * 1024 * 8) /* 50 KiB/s */

struct HWFrame
{
  USHORT hwf_Size;
  /* use layout of ethernet header here */
  UBYTE hwf_DstAddr[HW_ADDRFIELDSIZE];
  UBYTE hwf_SrcAddr[HW_ADDRFIELDSIZE];
  USHORT hwf_Type;
  /*UBYTE    hwf_Data[MTU];*/
};

/* ----- config stuff ----- */

struct PLIPBase;

/* return value for hw_handle_special_cmd() */
#define HW_SPECIAL_CMD_OK 0
#define HW_SPECIAL_CMD_ERROR 1
#define HW_SPECIAL_CMD_PARAM_CHANGE 2

/* hw events */
#define HW_EVENT_NONE           0
#define HW_EVENT_RX_PENDING     1
#define HW_EVENT_LINK_CHANGE    2
#define HW_EVENT_NEED_REINIT    4

/* hw API */
REGARGS BOOL hw_base_alloc(struct PLIPBase *pb);
REGARGS void hw_base_free(struct PLIPBase *pb);

REGARGS BOOL hw_init(struct PLIPBase *pb);
REGARGS BOOL hw_reinit(struct PLIPBase *pb);
REGARGS void hw_cleanup(struct PLIPBase *pb);

REGARGS BOOL hw_get_macs(struct PLIPBase *pb, UBYTE *cur_mac, UBYTE *def_mac);
REGARGS BOOL hw_set_mac(struct PLIPBase *pb, UBYTE *cur_mac);

REGARGS BOOL hw_can_handle_special_cmd(struct PLIPBase *pb, UWORD cmd);
REGARGS int hw_handle_special_cmd(struct PLIPBase *pb, struct IOSana2Req *req, BOOL offline);

REGARGS BOOL hw_attach(struct PLIPBase *pb);
REGARGS void hw_detach(struct PLIPBase *pb);

REGARGS void hw_get_sys_time(struct PLIPBase *pb, struct timeval *time);
REGARGS void hw_get_eclock(struct PLIPBase *pb, S2QUAD *quad);

REGARGS BOOL hw_send_frame(struct PLIPBase *pb, struct HWFrame *frame);
REGARGS BOOL hw_recv_frame(struct PLIPBase *pb, struct HWFrame *frame);

REGARGS BOOL hw_get_link_status(struct PLIPBase *pb);

REGARGS ULONG hw_get_event_sigmask(struct PLIPBase *pb);
REGARGS ULONG hw_get_extra_sigmask(struct PLIPBase *pb);
REGARGS BOOL  hw_is_event_pending(struct PLIPBase *pb);
REGARGS UWORD hw_handle_event_signal(struct PLIPBase *pb, BOOL from_wait);
REGARGS UWORD hw_handle_extra_signal(struct PLIPBase *pb);

REGARGS void hw_config_init(struct PLIPBase *pb,
                                   STRPTR *template_str,
                                   struct CommonConfig **cfg,
                                   STRPTR *config_file);
REGARGS void hw_config_update(struct PLIPBase *pb);
REGARGS void hw_config_dump(struct PLIPBase *pb);

#endif
