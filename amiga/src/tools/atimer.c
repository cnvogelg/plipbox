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

atimer_handle_t *atimer_init(struct Library *SysBase)
{
  /* alloc handle */
  struct atimer_handle *th;
  th = AllocMem(sizeof(struct atimer_handle), MEMF_ANY | MEMF_CLEAR | MEMF_PUBLIC);
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
      /* all ok */
      return th;
    }
  }

  /* something failed */
  atimer_exit(th);
  return NULL;
}

#define SysBase th->sysBase
#define TimerBase th->timerBase

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

void atimer_get_sys_time(atimer_handle_t *th, atime_stamp_t *val)
{
  GetSysTime((struct timeval *)val);
}

ULONG atimer_eclock_get(atimer_handle_t *th, atime_stamp_t *val)
{
  struct EClockVal *eval = (struct EClockVal *)val;
  th->eClockFreq = ReadEClock(eval);
  return th->eClockFreq;
}

void atimer_eclock_delta(atime_stamp_t *end, atime_stamp_t *begin, atime_stamp_t *delta)
{
  if (end->lo < begin->lo)
  {
    delta->lo = 0xffffffffUL - begin->lo + end->lo + 1;
    delta->hi = end->hi - begin->hi - 1;
  }
  else
  {
    delta->lo = end->lo - begin->lo;
    delta->hi = end->hi - begin->hi;
  }
}

ULONG atimer_eclock_to_us(atimer_handle_t *th, ULONG delta)
{
  return (delta * 1000000UL) / th->eClockFreq;
}

ULONG atimer_eclock_to_kBps(atimer_handle_t *th, ULONG delta, ULONG bytes)
{
  return (bytes * th->eClockFreq) / (delta * 1024UL);
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
