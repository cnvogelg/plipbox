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
#include "devices/plipbox.h"

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

GLOBAL REGARGS BOOL hw_get_macs(struct PLIPBase *pb, UBYTE *cur_mac, UBYTE *def_mac)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   int ok = proto_cmd_get_mac(hwb->proto, cur_mac);
   if(ok == PROTO_RET_OK) {
      ok = proto_cmd_get_def_mac(hwb->proto, def_mac);
   }
   return ok == PROTO_RET_OK;
}

GLOBAL REGARGS BOOL hw_can_handle_special_cmd(struct PLIPBase *pb, UWORD cmd)
{
  switch(cmd) {
    case S2PB_SET_MAC:
    case S2PB_SET_MODE:
    case S2PB_GET_MODE:
      return TRUE;
    default:
      return FALSE;
  }
}

GLOBAL REGARGS void hw_handle_special_cmd(struct PLIPBase *pb, struct IOSana2Req *req, BOOL offline)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   int res = PROTO_RET_OK;

   /* commands need offline mode */
   if(!offline) {
      req->ios2_Req.io_Error = S2ERR_BAD_STATE;
      req->ios2_WireError = S2WERR_UNIT_ONLINE;
      return;
   }

   switch(req->ios2_Req.io_Command) {
    case S2PB_SET_MAC:
      res = proto_cmd_set_mac(hwb->proto, req->ios2_SrcAddr);
      /* read and update mac */
      if(res == PROTO_RET_OK) {
        mac_t mac;
        res = proto_cmd_get_mac(hwb->proto, mac);
        if(res == PROTO_RET_OK) {
          CopyMem(mac, pb->pb_CfgAddr, MAC_SIZE);
        }
      }
      break;
    case S2PB_SET_MODE:
      res = proto_cmd_set_mode(hwb->proto, (UWORD)req->ios2_WireError);
      break;
    case S2PB_GET_MODE: {
      UWORD mode;
      res = proto_cmd_get_mode(hwb->proto, &mode);
      req->ios2_WireError = mode;
      break;
    }
    default:
      break;
   }

   /* update error in req */
   if(res != PROTO_RET_OK) {
      req->ios2_Req.io_Error = S2ERR_TX_FAILURE;
      req->ios2_WireError = S2WERR_GENERIC_ERROR;
   }
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
   UWORD status = 0;
   int ok = proto_cmd_send_frame(hwb->proto, buf, frame->hwf_Size, &status);
   if(ok == PROTO_RET_OK) {
      /* device transfer ok... check status */
      hwb->hwb_DeviceStatus = status;
      if(status & PROTO_CMD_STATUS_TX_ERROR) {
        return FALSE;
      } else {
        return TRUE;
      }
   } else {
      return FALSE;
   }
}

GLOBAL REGARGS BOOL hw_recv_frame(struct PLIPBase *pb, struct HWFrame *frame)
{
   ULONG pkttyp;
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   UBYTE *buf = ((UBYTE *)frame) + 2;
   UWORD status = 0;
   int ok = proto_cmd_recv_frame(hwb->proto, buf, HW_ETH_FRAME_SIZE, &frame->hwf_Size, &status);
   if(ok == PROTO_RET_OK) {
      /* device transfer ok... check status */
      hwb->hwb_DeviceStatus = status;
      if(status & PROTO_CMD_STATUS_RX_ERROR) {
        return FALSE;
      } else {
        return TRUE;
      }
   } else {
      return FALSE;
   }
}

GLOBAL REGARGS ULONG hw_status_get_sigmask(struct PLIPBase *pb)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   ULONG sigmask = proto_env_get_trigger_sigmask(hwb->env);
   d(("hw_status_get_sigmask: %lx\n", sigmask));
   return sigmask;
}

GLOBAL REGARGS BOOL hw_status_is_rx_pending(struct PLIPBase *pb)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   UWORD status = hwb->hwb_DeviceStatus;
   d(("hw_status_is_rx_pending: status=%lx\n", (ULONG)status));
   return (status & PROTO_CMD_STATUS_RX_PENDING) == PROTO_CMD_STATUS_RX_PENDING;
}

GLOBAL REGARGS BOOL hw_status_update(struct PLIPBase *pb)
{
   struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
   int ok;
   UWORD status;

   ok = proto_cmd_get_status(hwb->proto, &status);
   d(("hw_status_update: status=%lx ok=%lx", (ULONG)status, (ULONG)ok));
   if(ok != PROTO_RET_OK) {
      return FALSE;
   }

   hwb->hwb_DeviceStatus = status;
   return TRUE;
}
