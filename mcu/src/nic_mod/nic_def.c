#include "types.h"

#ifdef DEBUG_MODE
#define DEBUG
#endif

#include "debug.h"
#include "nic_def.h"
#include "nic_mod.h"
#include "nic_loop.h"

#ifdef HAVE_CYW43
#include "nic_cyw43.h"
#endif
#ifdef HAVE_ENC28J60
#include "nic_enc28j60.h"
#endif

// the table of available modes
const nic_mod_ptr_t ROM_ATTR nic_defs[] = {
#ifdef HAVE_CYW43
  &nic_mod_cyw43,
#endif
#ifdef HAVE_ENC28J60
  &nic_mod_enc28j60,
#endif
  &nic_mod_loop
};

const size_t nic_defs_size = (sizeof(nic_defs) / sizeof(nic_mod_ptr_t));
