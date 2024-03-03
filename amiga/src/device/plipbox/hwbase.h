/* hwbase.h - hardware specific part of driver structure */
#ifndef HWBASE_H
#define HWBASE_H

#include "proto_atom.h"
#include "proto_env.h"

/* ----- base structure for hardware ----- */

/* structure to be filled by ReadArgs template */
struct TemplateConfig
{
  struct CommonConfig common;
  ULONG *io_timeout;
  ULONG *tick_time;
};

struct HWBase
{
  proto_env_handle_t *env;
  proto_handle_t *proto;
  timer_handle_t *timer;
  UWORD token;

  /* internal state */
  UWORD hw_status;
  UWORD hw_event;
  ULONG num_rx;
  ULONG num_tx;

  /* config options */
  ULONG io_timeout_s;
  ULONG io_timeout_us;
  ULONG tick_time_s;
  ULONG tick_time_us;

  struct TemplateConfig config;
};

/* ----- config ----- */

#define CONFIGFILE "ENV:SANA2/plipbox.config"
#define TEMPLATE "IO_TIMEOUT/K/N,TICK_TIME/K/N"

#endif
