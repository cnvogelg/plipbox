#ifndef PARAM_DEF_H
#define PARAM_DEF_H

#include "types.h"
#include "arch.h"
#include "param.h"

typedef struct {
  // first reserved entry is checksum
  u16   crc;

  mac_t mac_addr;
  u16   mode;
  u16   flags;
  u32   delay;
  ip_addr_t  ip_addr;
  ip_addr_t  net_mask;
} param_t;

extern const param_t ROM_ATTR default_param;
extern const param_def_t ROM_ATTR param_defs[];
extern const size_t param_defs_size;
extern param_t param;

#endif