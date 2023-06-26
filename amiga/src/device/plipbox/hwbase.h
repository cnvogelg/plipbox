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
   ULONG *timeout;
   ULONG no_burst;
};

struct HWBase
{
   proto_env_handle_t *        env;
   proto_handle_t *            proto;

   UWORD                       hwb_MaxFrameSize;

   /* config options */
   ULONG                       hwb_TimeOutMicros;
   ULONG                       hwb_TimeOutSecs;
   UWORD                       hwb_BurstMode;

   struct TemplateConfig       hwb_Config;
};

/* ----- config ----- */

#define CONFIGFILE "ENV:SANA2/plipbox.config"
#define TEMPLATE "TIMEOUT/K/N,NOBURST/S"

#endif
