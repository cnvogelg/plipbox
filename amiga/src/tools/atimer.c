#define __NOLIBBASE__
#include <proto/exec.h>
#include <proto/timer.h>

#include "debug.h"
#include "compiler.h"

#include "atimer.h"

struct atimer_handle
{
  struct Library *sysBase;
  struct MsgPort *timerPort;
  struct MsgPort *sigTimerPort;
  struct timerequest timerReq;
  struct timerequest sigTimerReq;
  struct Library *timerBase;
  ULONG eClockFreq;
  ULONG timerSigMask;
};

#define TimerBase th->timerBase

atimer_handle_t *atimer_init(struct Library *SysBase)
{
  /* alloc handle */
  struct atimer_handle *th;
  th = AllocMem(sizeof(struct atimer_handle), MEMF_CLEAR | MEMF_ANY);
  if (th == NULL)
  {
    return NULL;
  }
  th->sysBase = SysBase;

  /* create msg port for timer access */
  th->timerPort = CreateMsgPort();
  if (th->timerPort != NULL)
  {

    /* setup timer request */
    th->timerReq.tr_node.io_Message.mn_ReplyPort = th->timerPort;
    if (!OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest *)&th->timerReq, 0))
    {
      /* store timer base */
      th->timerBase = (struct Library *)th->timerReq.tr_node.io_Device;

      /* init eClockFreq */
      struct EClockVal dummy;
      th->eClockFreq = ReadEClock(&dummy);

      /* all ok */
      return th;
    }
  }

  /* something failed */
  atimer_exit(th);
  return NULL;
}

#define SysBase th->sysBase

void atimer_exit(atimer_handle_t *th)
{
  if (th == NULL)
  {
    return;
  }

  /* cleanup timer request */
  if (th->timerBase != NULL)
  {
    CloseDevice((struct IORequest *)&th->timerReq);
  }

  /* remove timer port */
  if (th->timerPort != NULL)
  {
    DeleteMsgPort(th->timerPort);
  }

  /* free handle */
  FreeMem(th, sizeof(atimer_handle_t));
}

void atimer_sys_time_get(atimer_handle_t *th, atime_stamp_t *val)
{
  GetSysTime((struct timeval *)val);
}

void atimer_sys_time_delta(atime_stamp_t *end, atime_stamp_t *begin, atime_stamp_t *delta)
{
  if (end->ms < begin->ms)
  {
    delta->ms = 999999UL - begin->ms + end->ms + 1;
    delta->s = end->s - begin->s - 1;
  }
  else
  {
    delta->ms = end->ms - begin->ms;
    delta->s = end->s - begin->s;
  }
}

ULONG atimer_eclock_freq(atimer_handle_t *th)
{
  return th->eClockFreq;
}

void atimer_eclock_get(atimer_handle_t *th, atime_estamp_t *val)
{
  struct EClockVal *eval = (struct EClockVal *)val;
  ReadEClock(eval);
}

void atimer_eclock_delta(atime_estamp_t *end, atime_estamp_t *begin, atime_estamp_t *delta)
{
  *delta = *end - *begin;
}

ULONG atimer_eclock_to_us(atimer_handle_t *th, atime_estamp_t *delta)
{
  return (ULONG)((*delta * 1000000UL) / th->eClockFreq);
}

ULONG atimer_eclock_to_bps(atimer_handle_t *th, atime_estamp_t *delta, ULONG bytes)
{
  if(*delta == 0) {
    return 0;
  }

  return (ULONG)( (bytes * (atime_estamp_t)th->eClockFreq) / *delta );
}

BOOL atimer_sig_init(struct atimer_handle *th)
{
  th->sigTimerPort = CreateMsgPort();
  if (th->sigTimerPort != NULL)
  {
    th->sigTimerReq.tr_node.io_Message.mn_ReplyPort = th->sigTimerPort;
    th->sigTimerReq.tr_node.io_Device = th->timerReq.tr_node.io_Device;
    th->sigTimerReq.tr_node.io_Unit = th->timerReq.tr_node.io_Unit;
    th->sigTimerReq.tr_node.io_Command = TR_ADDREQUEST;
    th->sigTimerReq.tr_node.io_Flags = 0;

    th->timerSigMask = 1UL << th->sigTimerPort->mp_SigBit;

    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

ULONG atimer_sig_get_mask(struct atimer_handle *th)
{
  if (th->sigTimerPort != NULL)
  {
    return 1 << th->sigTimerPort->mp_SigBit;
  }
  else
  {
    return 0;
  }
}

void atimer_sig_exit(struct atimer_handle *th)
{
  if (th->sigTimerPort != NULL)
  {
    DeleteMsgPort(th->sigTimerPort);
    th->sigTimerPort = NULL;
  }
}

void atimer_sig_start(struct atimer_handle *th, ULONG secs, ULONG micros)
{
  th->sigTimerReq.tr_time.tv_secs = secs;
  th->sigTimerReq.tr_time.tv_micro = micros;
  SetSignal(0, th->timerSigMask);
  SendIO((struct IORequest *)&th->sigTimerReq);
}

void atimer_sig_stop(struct atimer_handle *th)
{
  struct IORequest *req = (struct IORequest *)&th->sigTimerReq;
  if (!CheckIO(req))
  {
    AbortIO(req);
  }
  WaitIO(req);
}
