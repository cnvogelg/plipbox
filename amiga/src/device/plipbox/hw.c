/*
 * hw.c - hardware dependent part of driver
 */

#include <proto/exec.h>
#include <exec/exec.h>
#include <string.h>

#include "global.h"
#include "debug.h"
#include "compiler.h"
#include "hw.h"
#include "hwbase.h"
#include "proto_cmd.h"
#include "proto_cmd_shared.h"

GLOBAL REGARGS void hw_get_sys_time(struct PLIPBase *pb, struct timeval *time)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
  timer_handle_t *th = proto_env_get_timer(hwb->env);
  timer_get_sys_time(th,(time_stamp_t *)time);
}

#define PLIP_DEFTIMEOUT          (500*1000)
#define PLIP_MINTIMEOUT          500
#define PLIP_MAXTIMEOUT          (10000*1000)

GLOBAL REGARGS void hw_config_init(struct PLIPBase *pb,
                                   STRPTR *template_str,
                                   struct CommonConfig **cfg,
                                   STRPTR *config_file)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;

  hwb->hwb_TimeOutSecs = PLIP_DEFTIMEOUT / 1000000L;
  hwb->hwb_TimeOutMicros = PLIP_DEFTIMEOUT % 1000000L;
  hwb->hwb_BurstMode = 1;

  *template_str = (STRPTR)TEMPLATE;
  *config_file = (STRPTR)CONFIGFILE;
  *cfg = (struct CommonConfig *)&hwb->hwb_Config;
}

GLOBAL REGARGS void hw_config_update(struct PLIPBase *pb)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
  struct TemplateConfig *args = &hwb->hwb_Config;

  if (args->timeout) {
    LONG to = BOUNDS(*args->timeout, PLIP_MINTIMEOUT, PLIP_MAXTIMEOUT);
    hwb->hwb_TimeOutMicros = to % 1000000L;
    hwb->hwb_TimeOutSecs = to / 1000000L;
  }

  if(args->no_burst) {
    hwb->hwb_BurstMode = 0;
  }
}

GLOBAL REGARGS void hw_config_dump(struct PLIPBase *pb)
{
#if DEBUG & 1
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
#endif
  d(("timeOut %ld.%ld\n", hwb->hwb_TimeOutSecs, hwb->hwb_TimeOutMicros));
  d(("burstMode %ld\n", (ULONG)hwb->hwb_BurstMode));
}

GLOBAL REGARGS BOOL hw_init(struct PLIPBase *pb)
{
   BOOL rc = FALSE;
   int res = 0;
   struct HWBase *hwb = (struct HWBase *)AllocVec(sizeof(struct HWBase), MEMF_CLEAR|MEMF_ANY);
   pb->pb_HWBase = hwb;
   if(hwb == NULL) {
     d(("no memory!\n"));
     return FALSE;
   }

   /* open env */
   hwb->env = proto_env_init(pb->pb_SysBase, &res);
   if(hwb->env != NULL) {
      res = proto_env_init_events(hwb->env);
      if(res == PROTO_ENV_OK) {
         /* open proto */
         hwb->proto = proto_atom_init(hwb->env);
         if(hwb->proto != NULL) {
            d(("env & proto: OK\n"));
            rc = TRUE;
         } else {
            d(("ERROR opening proto!\n"));
         }
      } else {
         d(("ERROR init events\n"));
      }
   } else {
      d(("ERROR opening env!\n"));
   }
    
   return rc;
}

GLOBAL REGARGS VOID hw_cleanup(struct PLIPBase *pb)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;

   if(hwb->proto != NULL) {
      d(("proto exit.\n"));
      proto_atom_exit(hwb->proto);
      hwb->proto = NULL;
   }

   if(hwb->env != NULL) {
      d(("env events exit\n"));
      proto_env_exit_events(hwb->env);
      d(("env exit.\n"));
      proto_env_exit(hwb->env);
      hwb->env = NULL;
   }

   d(("free hw base\n"));
   FreeVec(hwb);
   pb->pb_HWBase = NULL;
}

/*
 * hwattach - setup hardware if device gets online
 */
GLOBAL REGARGS BOOL hw_attach(struct PLIPBase *pb)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   int ok = proto_cmd_attach(hwb->proto);
   return ok == PROTO_RET_OK;
}

/*
 * shutdown hardware if device gets offline
 */
GLOBAL REGARGS VOID hw_detach(struct PLIPBase *pb)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   proto_cmd_detach(hwb->proto);
}

GLOBAL REGARGS BOOL hw_send_frame(struct PLIPBase *pb, struct HWFrame *frame)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   UBYTE *buf = ((UBYTE *)frame) + 2;
   int ok = proto_cmd_send_frame(hwb->proto, buf, frame->hwf_Size);
   return ok == PROTO_RET_OK;
}

GLOBAL REGARGS BOOL hw_recv_pending(struct PLIPBase *pb)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   int ok;
   UWORD status;

   ok = proto_cmd_get_status(hwb->proto, &status);
   if(ok != PROTO_RET_OK) {
      return FALSE;
   }

   return (status & PROTO_CMD_STATUS_RX_PENDING) == PROTO_CMD_STATUS_RX_PENDING;
}

GLOBAL REGARGS BOOL hw_recv_frame(struct PLIPBase *pb, struct HWFrame *frame)
{
   ULONG pkttyp;
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   UBYTE *buf = ((UBYTE *)frame) + 2;
   int ok = proto_cmd_recv_frame(hwb->proto, buf, HW_ETH_FRAME_SIZE, &frame->hwf_Size);
   return ok == PROTO_RET_OK;
}

GLOBAL REGARGS ULONG hw_recv_sigmask(struct PLIPBase *pb)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   ULONG sigmask = proto_env_get_trigger_sigmask(hwb->env);
   d(("hw_recv_sigmask: %lx\n", sigmask));
   return sigmask;
}
