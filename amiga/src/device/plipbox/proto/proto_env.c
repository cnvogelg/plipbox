#define __NOLIBBASE__
#include <proto/exec.h>

#include "compiler.h"
#include "debug.h"

#include "timer.h"
#include "pario.h"
#include "proto_env.h"

struct proto_env_handle {
  struct pario_handle *pario;
  struct timer_handle *timer;
  struct Library *sys_base;
  ULONG  ack_irq_sigmask;
  ULONG  timer_sigmask;
  BYTE   ack_irq_signal;
  BYTE   timer_signal;
};

proto_env_handle_t *proto_env_init(struct Library *SysBase, int *res)
{
  proto_env_handle_t *ph = AllocMem(sizeof(proto_env_handle_t), MEMF_CLEAR | MEMF_PUBLIC);
  if(ph == NULL) {
    return NULL;
  }

  ph->pario = NULL;
  ph->timer = NULL;
  ph->sys_base = SysBase;

  ph->pario = pario_init(SysBase);
  if(ph->pario == NULL) {
    *res = PROTO_ENV_ERROR_INIT_PARIO;
    return NULL;
  }

  ph->timer = timer_init(SysBase);
  if(ph->timer == NULL) {
    pario_exit(ph->pario);
    ph->pario = NULL;

    *res = PROTO_ENV_ERROR_INIT_TIMER;
    return NULL;
  }

  *res = PROTO_ENV_OK;
  return ph;
}

#undef SysBase
#define SysBase ph->sys_base

void proto_env_exit(proto_env_handle_t *ph)
{
  if(ph->timer != NULL) {
    timer_exit(ph->timer);
    ph->timer = NULL;
  }

  if(ph->pario != NULL) {
    pario_exit(ph->pario);
    ph->pario = NULL;
  }

  FreeMem(ph, sizeof(proto_env_handle_t));
}

pario_handle_t *proto_env_get_pario(proto_env_handle_t *ph)
{
  return ph->pario;
}

timer_handle_t *proto_env_get_timer(proto_env_handle_t *ph)
{
  return ph->timer;
}

struct Library *proto_env_get_sysbase(proto_env_handle_t *ph)
{
  return ph->sys_base;
}

int proto_env_init_events(proto_env_handle_t *ph)
{
  ph->timer_signal = -1;

  /* alloc ack signal */
  ph->ack_irq_signal = AllocSignal(-1);
  if(ph->ack_irq_signal == -1) {
    return PROTO_ENV_ERROR_NO_SIGNAL;
  }

  /* setup ack irq handler */
  struct Task *task = FindTask(NULL);
  int error = pario_setup_ack_irq(ph->pario, task, ph->ack_irq_signal);
  if(error) {
    FreeSignal(ph->ack_irq_signal);
    ph->ack_irq_signal = -1;
    return PROTO_ENV_ERROR_ACK_IRQ;
  }

  /* setup signal timer */
  ph->timer_signal = timer_sig_init(ph->timer);
  if(ph->timer_signal == -1) {
    pario_cleanup_ack_irq(ph->pario);
    FreeSignal(ph->ack_irq_signal);
    ph->ack_irq_signal = -1;
    return PROTO_ENV_ERROR_TIMER_SIGNAL;
  }

  ph->ack_irq_sigmask = 1 << ph->ack_irq_signal;
  ph->timer_sigmask = 1 << ph->timer_signal;

  return PROTO_ENV_OK;
}

void proto_env_exit_events(proto_env_handle_t *ph)
{
  if(ph->timer_signal != -1) {
    /* timer cleanup */
    timer_sig_exit(ph->timer);
    ph->timer_signal = -1;
  }

  if(ph->ack_irq_signal != -1) {
    /* cleanup ack irq */
    pario_cleanup_ack_irq(ph->pario);

    /* free signal */
    FreeSignal(ph->ack_irq_signal);
    ph->ack_irq_signal = -1;
  }
}

ULONG proto_env_wait_event(proto_env_handle_t *ph,
                        ULONG timeout_s, ULONG timeout_us, ULONG extra_sigmask)
{
  /* wait for either timeout or ack */
  ULONG ack_mask = ph->ack_irq_sigmask;
  ULONG mask = ack_mask | ph->timer_sigmask | extra_sigmask;
  timer_sig_start(ph->timer, timeout_s, timeout_us);
  ULONG got = Wait(mask);
  timer_sig_stop(ph->timer);

  // confirm ack irq
  if(got & ack_mask) {
    pario_confirm_ack_irq(ph->pario);
  }

  return got;
}

void proto_env_confirm_trigger(proto_env_handle_t *ph)
{
  pario_confirm_ack_irq(ph->pario);
}

ULONG proto_env_get_trigger_sigmask(proto_env_handle_t *ph)
{
  return ph->ack_irq_sigmask;
}

ULONG proto_env_get_timer_sigmask(proto_env_handle_t *ph)
{
  return ph->timer_sigmask;
}

UWORD proto_env_get_num_triggers(proto_env_handle_t *ph)
{
  return pario_get_ack_irq_counter(ph->pario);
}

UWORD proto_env_get_num_trigger_signals(proto_env_handle_t *ph)
{
  return pario_get_signal_counter(ph->pario);
}

const char *proto_env_perror(int res)
{
  switch(res) {
    case PROTO_ENV_OK:
      return "OK";
    case PROTO_ENV_ERROR_INIT_PARIO:
      return "pario init failed";
    case PROTO_ENV_ERROR_INIT_TIMER:
      return "timer init failed";
    case PROTO_ENV_ERROR_NO_SIGNAL:
      return "can't alloc signal";
    case PROTO_ENV_ERROR_ACK_IRQ:
      return "ack irq setup failed";
    case PROTO_ENV_ERROR_TIMER_SIGNAL:
      return "timer signal failed";
    default:
      return "?";
  }
}
