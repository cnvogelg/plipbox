#define __NOLIBBASE__
#include <proto/exec.h>
#include <proto/timer.h>

#include "debug.h"
#include "compiler.h"

#include "timer.h"

struct timer_handle
{
  struct Library *sysBase;
  ULONG initFlags;
  struct MsgPort *timerPort;
  struct MsgPort *sigTimerPort;
  struct Task *task;
  APTR oldExceptCode;
  APTR oldExceptData;
  ULONG oldExcept;
  struct timerequest timerReq;
  struct timerequest sigTimerReq;
  struct Library *timerBase;
  ULONG timerSigMask;
  ULONG eClockFreq;
  ULONG syncSigMask;
  BYTE syncSig;
  volatile UBYTE timeoutFlag;
};

static ULONG ASM SAVEDS exc_handler(REG(d0, ULONG sigmask), REG(a1, struct timer_handle *th));

struct timer_handle *timer_init(struct Library *SysBase)
{
  /* alloc handle */
  struct timer_handle *th;
  th = AllocMem(sizeof(struct timer_handle), MEMF_CLEAR | MEMF_PUBLIC);
  if (th == NULL)
  {
    return NULL;
  }
  th->sysBase = SysBase;

  /* create msg port for timer access */
  th->timerPort = CreateMsgPort();
  if (th->timerPort != NULL)
  {
    th->initFlags = 1;

    /* setup new exception data */
    th->task = FindTask(NULL);
    th->oldExcept = SetExcept(0, 0xffffffff); /* turn'em off */
    th->oldExceptCode = th->task->tc_ExceptCode;
    th->oldExceptData = th->task->tc_ExceptData;
    th->task->tc_ExceptCode = (APTR)&exc_handler;
    th->task->tc_ExceptData = th;

    /* set timer signal to cause exceptions */
    ULONG sigmask = 1 << th->timerPort->mp_SigBit;
    SetSignal(0, sigmask);
    SetExcept(sigmask, sigmask);
    th->initFlags |= 2;

    /* setup timer request */
    th->timerReq.tr_node.io_Message.mn_ReplyPort = th->timerPort;
    if (!OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest *)&th->timerReq, 0))
    {
      /* store timer base */
      th->timerBase = (struct Library *)th->timerReq.tr_node.io_Device;

      /* prepare request */
      th->timerReq.tr_node.io_Flags = IOF_QUICK;
      th->timerReq.tr_node.io_Command = TR_ADDREQUEST;
      th->initFlags |= 4;

      /* preset flag */
      th->timeoutFlag = 0xff;

      /* get signal */
      th->syncSig = AllocSignal(-1);
      if (th->syncSig != -1)
      {
        th->syncSigMask = 1 << th->syncSig;
        th->initFlags |= 8;

        /* all ok */
        return th;
      }
    }
  }

  /* something failed */
  timer_exit(th);
  return NULL;
}

#define SysBase th->sysBase
#define TimerBase th->timerBase

void timer_exit(struct timer_handle *th)
{
  if (th == NULL)
  {
    return;
  }

  /* free signal */
  if (th->initFlags & 8)
  {
    FreeSignal(th->syncSig);
  }

  /* cleanup timer request */
  if (th->initFlags & 4)
  {
    struct IORequest *req = (struct IORequest *)&th->timerReq;
    WaitIO(req);
    CloseDevice(req);
  }

  /* cleanup task's exception state */
  if (th->initFlags & 2)
  {
    SetExcept(0, 0xffffffff); /* turn'em off */
    th->task->tc_ExceptCode = th->oldExceptCode;
    th->task->tc_ExceptData = th->oldExceptData;
    SetExcept(th->oldExcept, 0xffffffff);
  }

  /* remove timer port */
  if (th->initFlags & 1)
  {
    DeleteMsgPort(th->timerPort);
  }

  /* free handle */
  FreeMem(th, sizeof(struct timer_handle));
}

volatile UBYTE *timer_get_flag(struct timer_handle *th)
{
  return &th->timeoutFlag;
}

void timer_start(struct timer_handle *th, ULONG secs, ULONG micros)
{
  /* wait for exc handler */
  while (!th->timeoutFlag)
  {
    Wait(th->syncSigMask);
  };

  /* clear timeout flag */
  th->timeoutFlag = 0;
  SetSignal(0, th->syncSigMask);

  /* start new timeout timer */
  th->timerReq.tr_time.tv_secs = secs;
  th->timerReq.tr_time.tv_micro = micros;
  SendIO((struct IORequest *)&th->timerReq);
}

void timer_stop(struct timer_handle *th)
{
  struct IORequest *req = (struct IORequest *)&th->timerReq;
  /* stop timeout timer */
  if (!CheckIO(req))
  {
    AbortIO(req);
  }
  WaitIO(req);
}

void timer_get_sys_time(struct timer_handle *th, time_stamp_t *val)
{
  GetSysTime((struct timeval *)val);
}

ULONG timer_eclock_get(struct timer_handle *th, time_stamp_t *val)
{
  struct EClockVal *eval = (struct EClockVal *)val;
  th->eClockFreq = ReadEClock(eval);
  return th->eClockFreq;
}

void timer_eclock_delta(time_stamp_t *end, time_stamp_t *begin, time_stamp_t *delta)
{
  *delta = *end - *begin;
}

ULONG timer_eclock_to_us(struct timer_handle *th, time_stamp_t *delta)
{
  return (ULONG)(*delta * 1000000UL / th->eClockFreq);
}

union conv
{
  time_stamp_t ts;
  struct
  {
    ULONG hi;
    ULONG lo;
  } hilo;
};

void timer_eclock_split(time_stamp_t *val, ULONG *hi, ULONG *lo)
{
  union conv c;
  c.ts = *val;
  *hi = c.hilo.hi;
  *lo = c.hilo.lo;
}

static ULONG ASM SAVEDS exc_handler(REG(d0, ULONG sigmask), REG(a1, struct timer_handle *th))
{
  /* remove the I/O Block from the port */
  WaitIO((struct IORequest *)&th->timerReq);

  /* set timeout flag */
  th->timeoutFlag = 0xff;

  /* send sync signal */
  Signal(th->task, th->syncSigMask);

  /* re-enable signal */
  return sigmask;
}

BOOL timer_sig_init(struct timer_handle *th)
{
  th->sigTimerPort = CreateMsgPort();
  if (th->sigTimerPort != NULL)
  {
    th->initFlags |= 16;

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

ULONG timer_sig_get_mask(struct timer_handle *th)
{
  return 1 << th->sigTimerPort->mp_SigBit;
}

void timer_sig_exit(struct timer_handle *th)
{
  if (th->initFlags & 16)
  {
    th->initFlags &= ~16;

    DeleteMsgPort(th->sigTimerPort);
  }
}

void timer_sig_start(struct timer_handle *th, ULONG secs, ULONG micros)
{
  th->sigTimerReq.tr_time.tv_secs = secs;
  th->sigTimerReq.tr_time.tv_micro = micros;
  SetSignal(0, th->timerSigMask);
  SendIO((struct IORequest *)&th->sigTimerReq);
}

void timer_sig_stop(struct timer_handle *th)
{
  struct IORequest *req = (struct IORequest *)&th->sigTimerReq;
  if (!CheckIO(req))
  {
    AbortIO(req);
  }
  WaitIO(req);
}
