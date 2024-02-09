#include "types.h"

#ifdef DEBUG_MODE
#define DEBUG
#endif

#include "debug.h"
#include "nic_def.h"
#include "nic_mod.h"
#include "nic_loop.h"
#include "nic_enc28j60.h"

// the table of available modes
const nic_mod_ptr_t ROM_ATTR nic_defs[] = {
  &nic_mod_loop,
  &nic_mod_enc28j60
};

const size_t nic_defs_size = (sizeof(nic_defs) / sizeof(nic_mod_ptr_t));
