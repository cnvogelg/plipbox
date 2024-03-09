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

static REGARGS ULONG decode_hw_event(ULONG hw_status, ULONG last_status);

GLOBAL REGARGS void hw_get_sys_time(struct PLIPBase *pb, struct timeval *time)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
  timer_handle_t *th = proto_env_get_timer(hwb->env);
  timer_get_sys_time(th, (time_stamp_t *)time);
}

GLOBAL REGARGS void hw_get_eclock(struct PLIPBase *pb, S2QUAD *quad)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
  if(hwb == NULL) {
    return;
  }
  timer_handle_t *th = proto_env_get_timer(hwb->env);
  if(th == NULL) {
    return;
  }
  timer_eclock_get(th, (time_stamp_t *)quad);
}

#define IO_TIMEOUT_DEF (500 * 1000)   // 500ms
#define IO_TIMEOUT_MIN 500            // 500us
#define IO_TIMEOUT_MAX (10000 * 1000) // 10s

#define TICK_TIME_DEF 5000000UL  // 5s
#define TICK_TIME_MIN 100000UL   // 100ms
#define TICK_TIME_MAX 60000000UL // 60s

GLOBAL REGARGS BOOL hw_base_alloc(struct PLIPBase *pb)
{
  struct HWBase *hwb = (struct HWBase *)AllocVec(sizeof(struct HWBase), MEMF_CLEAR | MEMF_ANY);
  pb->pb_HWBase = hwb;
  return (hwb != NULL);
}

GLOBAL REGARGS void hw_base_free(struct PLIPBase *pb)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
  if (hwb != NULL)
  {
    FreeVec(hwb);
    pb->pb_HWBase = NULL;
  }
}

GLOBAL REGARGS void hw_config_init(struct PLIPBase *pb,
                                   STRPTR *template_str,
                                   struct CommonConfig **cfg,
                                   STRPTR *config_file)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;

  hwb->io_timeout_s = IO_TIMEOUT_DEF / 1000000UL;
  hwb->io_timeout_us = IO_TIMEOUT_DEF % 1000000UL;

  hwb->tick_time_s = TICK_TIME_DEF / 1000000UL;
  hwb->tick_time_us = TICK_TIME_DEF % 1000000UL;

  *template_str = (STRPTR)TEMPLATE;
  *config_file = (STRPTR)CONFIGFILE;
  *cfg = (struct CommonConfig *)&hwb->config;
}

GLOBAL REGARGS void hw_config_update(struct PLIPBase *pb)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
  struct TemplateConfig *args = &hwb->config;

  if (args->io_timeout)
  {
    ULONG to = BOUNDS(*args->io_timeout, IO_TIMEOUT_MIN, IO_TIMEOUT_MAX);
    hwb->io_timeout_s = to / 1000000L;
    hwb->io_timeout_us = to % 1000000L;
  }
  if (args->tick_time)
  {
    ULONG to = BOUNDS(*args->tick_time, TICK_TIME_MIN, TICK_TIME_MAX);
    hwb->tick_time_s = to / 1000000L;
    hwb->tick_time_us = to % 1000000L;
  }
}

GLOBAL REGARGS void hw_config_dump(struct PLIPBase *pb)
{
#if DEBUG & 1
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
#endif
  d(("IO_TIMEOUT %ld.%ld\n", hwb->io_timeout_s, hwb->io_timeout_us));
  d(("TICK_TIME  %ld.%ld\n", hwb->tick_time_s, hwb->tick_time_us));
}

GLOBAL REGARGS BOOL hw_init(struct PLIPBase *pb)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
  int res = 0;

  // open env
  hwb->env = proto_env_init(pb->pb_SysBase, &res);
  if (hwb->env == NULL)
  {
    d(("ERROR: proto env failed! res=%ld\n", (LONG)res));
    return FALSE;
  }

  // init events
  res = proto_env_init_events(hwb->env);
  if (res != PROTO_ENV_OK)
  {
    d(("ERROR: proto env init events failed! res=%ld\n", (LONG)res));
    return FALSE;
  }

  // setup timer signal
  timer_handle_t *timer = proto_env_get_timer(hwb->env);
  if (!timer_sig_init(timer))
  {
    d(("ERROR: timer setup!\n"));
    return FALSE;
  }
  hwb->timer = timer;

  // proto atom init
  hwb->proto = proto_atom_init(hwb->env, hwb->io_timeout_s, hwb->io_timeout_us);
  if (hwb->proto == NULL)
  {
    d(("ERROR: opening proto!\n"));
    return FALSE;
  }

  // fire tick timer
  d(("start tick timer!\n"));
  timer_sig_start(hwb->timer, hwb->tick_time_s, hwb->tick_time_us);
  hwb->num_rx = 0;
  hwb->num_tx = 0;

  d(("init: OK!\n"));

  // do the reinit part
  return hw_reinit(pb);
}

GLOBAL REGARGS BOOL hw_reinit(struct PLIPBase *pb)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;

  d(("reinit:\n"));

  // set non-null (random) driver token
  time_stamp_t ts;
  timer_get_sys_time(hwb->timer, &ts);
  hwb->token = (UWORD)(ts & 0xffffUL);

  // send INIT with token to device
  // check if device reacts to proto protocol and setup token on device
  // the token allows to check later on with PING if the device was reset/or not
  d(("send INIT: token=%lx\n", (ULONG)hwb->token));
  int res = proto_cmd_init(hwb->proto, hwb->token);
  if (res != PROTO_RET_OK)
  {
    d(("ERROR: cmd init failed! ret=%ld\n", (LONG)res));
    return FALSE;
  }

  d(("reinit:OK\n"));
  return TRUE;
}

GLOBAL REGARGS VOID hw_cleanup(struct PLIPBase *pb)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
  if (hwb == NULL)
  {
    return;
  }

  if (hwb->timer != NULL)
  {
    d(("timer stop.\n"));
    timer_sig_stop(hwb->timer);
    d(("timer exit.\n"));
    timer_sig_exit(hwb->timer);
  }

  if (hwb->proto != NULL)
  {
    d(("send EXIT\n"));
    proto_cmd_exit(hwb->proto);

    d(("proto exit.\n"));
    proto_atom_exit(hwb->proto);
    hwb->proto = NULL;
  }

  if (hwb->env != NULL)
  {
    d(("env events exit\n"));
    proto_env_exit_events(hwb->env);
    d(("env exit.\n"));
    proto_env_exit(hwb->env);
    hwb->env = NULL;
  }
}

GLOBAL REGARGS BOOL hw_get_macs(struct PLIPBase *pb, UBYTE *cur_mac, UBYTE *def_mac)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
  int ok = proto_cmd_get_cur_mac(hwb->proto, cur_mac);
  if (ok == PROTO_RET_OK)
  {
    ok = proto_cmd_get_def_mac(hwb->proto, def_mac);
  }
  return ok == PROTO_RET_OK;
}

GLOBAL REGARGS BOOL hw_set_mac(struct PLIPBase *pb, UBYTE *cur_mac)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
  return proto_cmd_set_cur_mac(hwb->proto, cur_mac);
}

GLOBAL REGARGS BOOL hw_can_handle_special_cmd(struct PLIPBase *pb, UWORD cmd)
{
  /* register all special comamnds here! */
  switch (cmd)
  {
  case S2PB_GET_VERSION:
  case S2PB_PARAM_GET_NUM:
  case S2PB_PARAM_FIND_TAG:
  case S2PB_PARAM_GET_DEF:
  case S2PB_PARAM_GET_VAL:
  case S2PB_PARAM_SET_VAL:
  case S2PB_PREFS_RESET:
  case S2PB_PREFS_LOAD:
  case S2PB_PREFS_SAVE:
    return TRUE;
  default:
    return FALSE;
  }
}

GLOBAL REGARGS int hw_handle_special_cmd(struct PLIPBase *pb, struct IOSana2Req *req, BOOL offline)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
  int res = PROTO_RET_OK;
  int return_value = HW_SPECIAL_CMD_OK;

  /* commands need offline mode */
  if (!offline)
  {
    req->ios2_Req.io_Error = S2ERR_BAD_STATE;
    req->ios2_WireError = S2WERR_UNIT_ONLINE;
    return HW_SPECIAL_CMD_ERROR;
  }

  switch (req->ios2_Req.io_Command)
  {
  case S2PB_GET_VERSION:
  {
    UWORD version = 0;
    res = proto_cmd_get_version(hwb->proto, &version);
    /* set both device and driver version */
    req->ios2_WireError = version | DEVICE_VERSION << 24 | DEVICE_REVISION << 16;
    break;
  }
  // ----- param -----
  case S2PB_PARAM_GET_NUM:
  {
    UWORD num_param = 0;
    res = proto_cmd_param_get_num(hwb->proto, &num_param);
    req->ios2_WireError = num_param;
    break;
  }
  case S2PB_PARAM_FIND_TAG:
  {
    ULONG tag = req->ios2_WireError;
    UWORD id = S2PB_NO_INDEX;
    res = proto_cmd_param_find_tag(hwb->proto, tag, &id);
    req->ios2_WireError = id;
    break;
  }
  case S2PB_PARAM_GET_DEF:
  {
    UWORD id = (UWORD)req->ios2_WireError;
    if (req->ios2_DataLength != sizeof(proto_param_def_t))
    {
      req->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
      req->ios2_WireError = S2WERR_GENERIC_ERROR;
    }
    else
    {
      proto_param_def_t *def = (proto_param_def_t *)req->ios2_Data;
      res = proto_cmd_param_get_def(hwb->proto, id, def);
    }
    break;
  }
  case S2PB_PARAM_GET_VAL:
  {
    UWORD id = (UWORD)req->ios2_WireError;
    UWORD size = (UWORD)req->ios2_DataLength;
    UBYTE *data = (UBYTE *)req->ios2_Data;
    res = proto_cmd_param_get_val(hwb->proto, id, size, data);
    break;
  }
  case S2PB_PARAM_SET_VAL:
  {
    UWORD id = (UWORD)req->ios2_WireError;
    UWORD size = (UWORD)req->ios2_DataLength;
    UBYTE *data = (UBYTE *)req->ios2_Data;
    res = proto_cmd_param_set_val(hwb->proto, id, size, data);
    return_value = HW_SPECIAL_CMD_PARAM_CHANGE;
    break;
  }
  // ----- prefs -----
  case S2PB_PREFS_RESET:
    res = proto_cmd_prefs_reset(hwb->proto);
    return_value = HW_SPECIAL_CMD_PARAM_CHANGE;
    break;
  case S2PB_PREFS_LOAD:
  {
    UWORD status = 0;
    res = proto_cmd_prefs_load(hwb->proto, &status);
    req->ios2_WireError = status;
    return_value = HW_SPECIAL_CMD_PARAM_CHANGE;
    break;
  }
  case S2PB_PREFS_SAVE:
  {
    UWORD status = 0;
    res = proto_cmd_prefs_save(hwb->proto, &status);
    req->ios2_WireError = status;
    break;
  }
  default:
    break;
  }

  /* update error in req */
  if (res != PROTO_RET_OK)
  {
    req->ios2_Req.io_Error = S2ERR_TX_FAILURE;
    req->ios2_WireError = S2WERR_GENERIC_ERROR;
    return HW_SPECIAL_CMD_ERROR;
  }
  else
  {
    return return_value;
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
  if (ok == PROTO_RET_OK)
  {
    /* update events? */
    hwb->hw_event = decode_hw_event(status, hwb->hw_status);
    hwb->hw_status = status;

    /* device transfer ok... check status */
    if (status & PROTO_CMD_STATUS_TX_ERROR)
    {
      return FALSE;
    }
    else
    {
      return TRUE;
    }
  }
  else
  {
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
  if (ok == PROTO_RET_OK)
  {
    /* update events? */
    hwb->hw_event = decode_hw_event(status, hwb->hw_status);
    hwb->hw_status = status;

    /* device transfer ok... check status */
    if (status & PROTO_CMD_STATUS_RX_ERROR)
    {
      return FALSE;
    }
    else
    {
      return TRUE;
    }
  }
  else
  {
    return FALSE;
  }
}

/* events and extra signals */

GLOBAL REGARGS ULONG hw_get_event_sigmask(struct PLIPBase *pb)
{
  /* the event signal is generated by the proto IRQ directly (namely the ACK)
   */
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
  ULONG sigmask = proto_env_get_trigger_sigmask(hwb->env);
  d(("hw_get_event_sigmask: %lx\n", sigmask));
  return sigmask;
}

GLOBAL REGARGS ULONG hw_get_extra_sigmask(struct PLIPBase *pb)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;
  timer_handle_t *timer = hwb->timer;
  ULONG sigmask = timer_sig_get_mask(timer);
  d(("hw_get_extra_sigmask: %lx\n", sigmask));
  return sigmask;
}

static REGARGS ULONG decode_hw_event(ULONG hw_status, ULONG last_status)
{
  // RX pending
  if ((hw_status & PROTO_CMD_STATUS_RX_PENDING) == PROTO_CMD_STATUS_RX_PENDING)
  {
    return HW_EVENT_RX_PENDING;
  }

  // check change to last status
  ULONG change = hw_status ^ last_status;
  if((change & PROTO_CMD_STATUS_LINK_UP) == PROTO_CMD_STATUS_LINK_UP) {
    // link change!
    if((hw_status & PROTO_CMD_STATUS_LINK_UP) == PROTO_CMD_STATUS_LINK_UP) {
      return HW_EVENT_LINK_UP;
    }
    else {
      return HW_EVENT_LINK_DOWN;
    }
  }

  return FALSE;
}

static REGARGS BOOL poll_status(struct PLIPBase *pb, UWORD *hw_status)
{
  /* fetch status from device */
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;

  proto_env_confirm_trigger(hwb->env);

  int ok = proto_cmd_get_status(hwb->proto, hw_status);
  d(("poll_status: status=%lx ok=%lx", (ULONG)*hw_status, (ULONG)ok));
  return (ok == PROTO_RET_OK);
}

GLOBAL REGARGS BOOL hw_is_event_pending(struct PLIPBase *pb)
{
  /* hw_is_event_pending() will be called at various points
     in the driver to check if a hw event has to be handled. if yes then
     ops (like write) are skipped and it returns to the driver main loop
     to handle the event immediately with hw_handle_event_signal().
  */

  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;

  /* was a hw_event already detected? */
  if(hwb->hw_event != HW_EVENT_NONE) {
    d(("hw_is_event_pending: already got hw_event=%lx", (ULONG)hwb->hw_event));
    return TRUE;
  }

  /* we also poll th signal mask to check if the ACK irq has arrived till then */
  ULONG sigmask = proto_env_get_trigger_sigmask(hwb->env);
  ULONG gotmask = SetSignal(0, sigmask);
  if ((gotmask & sigmask) == sigmask)
  {
    UWORD new_status = 0;
    BOOL ok = poll_status(pb, &new_status);
    if(ok) {
      /* decode event from last status and new one */
      hwb->hw_event = decode_hw_event(new_status, hwb->hw_status);
      d(("hw_is_event_pending: polled hw_status=%lx new_status=%lx -> hw_event=%lx\n",
      (ULONG)hwb->hw_status, (ULONG)new_status, (ULONG)hwb->hw_event));

      /* some event is already pending */
      if(hwb->hw_event != HW_EVENT_NONE) {
        return TRUE;
      }
    }
  }

  /* nothing to do */
  return FALSE;
}

GLOBAL REGARGS UWORD hw_handle_event_signal(struct PLIPBase *pb, BOOL from_wait)
{
  /* this is called by the driver after receiving the event signal of the hw.
     it prepares the upcoming hw_is_event_pending() call
  */
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;

  /* need to poll status again */
  if(from_wait) {
    UWORD new_status = 0;
    BOOL ok = poll_status(pb, &new_status);
    if(ok) {
      /* decode event from last status and new one */
      hwb->hw_event = decode_hw_event(new_status, hwb->hw_status);
      d(("hw_handle_event_signal: polled hw_status=%lx new_status=%lx -> hw_event=%lx\n",
      (ULONG)hwb->hw_status, (ULONG)new_status, (ULONG)hwb->hw_event));
    }
  }

  /* return current events and reset event mask */
  UWORD result = hwb->hw_event;
  hwb->hw_event = 0;
  return result;
}

GLOBAL REGARGS UWORD hw_handle_extra_signal(struct PLIPBase *pb)
{
  struct HWBase *hwb = (struct HWBase *)pb->pb_HWBase;

  /* timer ticked */
  d(("timer!\n"));

  // was there activity during tick time? no... do a ping
  if ((hwb->num_rx == 0) && (hwb->num_tx == 0))
  {
    d(("ping!\n"));
    UWORD token = 0;
    int res = proto_cmd_ping(hwb->proto, &token);
    if (res != PROTO_RET_OK)
    {
      d(("ping FAILED!\n"));
      // ping failed ... hw has trouble...
      return HW_EVENT_NEED_REINIT;
    }
    // check token. changed? hw seems to be reset?
    if (token != hwb->token)
    {
      d(("ping token CHANGED! %lx != %lx\n", (ULONG)token, (ULONG)hwb->token));
      return HW_EVENT_NEED_REINIT;
    }
  }

  hwb->num_tx = 0;
  hwb->num_rx = 0;

  /* refire timer */
  timer_sig_start(hwb->timer, hwb->tick_time_s, hwb->tick_time_us);

  return HW_EVENT_NONE;
}
