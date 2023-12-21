#define __NOLIBBASE__
#include <proto/exec.h>

#include "compiler.h"
#include "debug.h"

#include "pario.h"
#include "timer.h"
#include "proto_low.h"
#include "proto_atom.h"

struct proto_handle {
    proto_env_handle_t  *penv;
    struct pario_port   *port;
    struct timer_handle *timer;
    ULONG                timeout_s;
    ULONG                timeout_us;
    struct Library      *sys_base;
};

proto_handle_t *proto_atom_init(proto_env_handle_t *penv, ULONG timeout_s, ULONG timeout_us)
{
  proto_handle_t *ph;
  struct Library *SysBase = (struct Library *)proto_env_get_sysbase(penv);

  ph = AllocMem(sizeof(struct proto_handle), MEMF_CLEAR | MEMF_PUBLIC);
  if(ph == NULL) {
    return NULL;
  }
  ph->penv = penv;
  ph->port = pario_get_port(proto_env_get_pario(penv));
  ph->timer = proto_env_get_timer(penv);
  ph->timeout_s  = timeout_s;
  ph->timeout_us = timeout_us;
  ph->sys_base = SysBase;

  proto_low_config_port(ph->port);

  return ph;
}

#undef SysBase
#define SysBase ph->sys_base

void proto_atom_exit(proto_handle_t *ph)
{
  if(ph == NULL) {
    return;
  }
  /* free handle */
  FreeMem(ph, sizeof(struct proto_handle));
}

proto_env_handle_t *proto_atom_get_env(proto_handle_t *ph)
{
  return ph->penv;
}

int proto_atom_action(proto_handle_t *ph, UBYTE cmd)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);

  timer_start(ph->timer, ph->timeout_s, ph->timeout_us);
  int result = proto_low_action(port, timeout_flag, cmd);
  timer_stop(ph->timer);

  return result;
}

int proto_atom_action_no_busy(proto_handle_t *ph, UBYTE cmd)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);

  timer_start(ph->timer, ph->timeout_s, ph->timeout_us);
  int result = proto_low_action_no_busy(port, timeout_flag, cmd);
  timer_stop(ph->timer);

  return result;
}

static ASM void bench_cb(REG(d0, int id), REG(a2, struct cb_data *cb))
{
  struct timer_handle *th = (struct timer_handle *)cb->user_data;
  time_stamp_t *ts = &cb->timestamps[id];
  timer_eclock_get(th, ts);
}

int proto_atom_action_bench(proto_handle_t *ph, UBYTE cmd, ULONG deltas[2])
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);

  struct cb_data cbd = {
    bench_cb,
    ph->timer
  };

  timer_start(ph->timer, ph->timeout_s, ph->timeout_us);
  int result = proto_low_action_bench(port, timeout_flag, &cbd, cmd);
  timer_stop(ph->timer);

  /* calc deltas */
  time_stamp_t *t0 = &cbd.timestamps[0];
  time_stamp_t *t1 = &cbd.timestamps[1];
  time_stamp_t *t2 = &cbd.timestamps[2];
  time_stamp_t d0, d1;
  timer_eclock_delta(t1, t0, &d0);
  timer_eclock_delta(t2, t1, &d1);
  deltas[0] = timer_eclock_to_us(ph->timer, &d0);
  deltas[1] = timer_eclock_to_us(ph->timer, &d1);

  return result;
}

int proto_atom_read_word(proto_handle_t *ph, UBYTE cmd, UWORD *data)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);

  timer_start(ph->timer, ph->timeout_s, ph->timeout_us);
  int result = proto_low_read_word(port, timeout_flag, cmd, data);
  timer_stop(ph->timer);

  return result;
}

int proto_atom_write_word(proto_handle_t *ph, UBYTE cmd, UWORD data)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);

  timer_start(ph->timer, ph->timeout_s, ph->timeout_us);
  int result = proto_low_write_word(port, timeout_flag, cmd, &data);
  timer_stop(ph->timer);

  return result;
}

int proto_atom_read_long(proto_handle_t *ph, UBYTE cmd, ULONG *data)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);

  timer_start(ph->timer, ph->timeout_s, ph->timeout_us);
  int result = proto_low_read_long(port, timeout_flag, cmd, data);
  timer_stop(ph->timer);

  return result;
}

int proto_atom_write_long(proto_handle_t *ph, UBYTE cmd, ULONG data)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);

  timer_start(ph->timer, ph->timeout_s, ph->timeout_us);
  int result = proto_low_write_long(port, timeout_flag, cmd, &data);
  timer_stop(ph->timer);

  return result;
}

int proto_atom_write_block(proto_handle_t *ph, UBYTE cmd, UBYTE *buf, UWORD num_bytes)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  if(num_bytes & 1) {
    return PROTO_RET_ODD_BLOCK_SIZE;
  }
  UWORD num_words = num_bytes >> 1;

  timer_start(ph->timer, ph->timeout_s, ph->timeout_us);
  int result = proto_low_write_block(port, timeout_flag, cmd, buf, num_words);
  timer_stop(ph->timer);

  return result;
}

int proto_atom_read_block(proto_handle_t *ph, UBYTE cmd, UBYTE *buf, UWORD num_bytes)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  if(num_bytes & 1) {
    return PROTO_RET_ODD_BLOCK_SIZE;
  }
  UWORD num_words = num_bytes >> 1;

  timer_start(ph->timer, ph->timeout_s, ph->timeout_us);
  int result = proto_low_read_block(port, timeout_flag, cmd, buf, num_words);
  timer_stop(ph->timer);

  return result;
}

// verbose error

const char *proto_atom_perror(int res)
{
  switch(res) {
    case PROTO_RET_OK:
      return "OK";
    case PROTO_RET_RAK_INVALID:
      return "RAK invalid";
    case PROTO_RET_TIMEOUT:
      return "timeout";
    case PROTO_RET_DEVICE_BUSY:
      return "device is busy";
    default:
      return "?";
  }
}
