#include "types.h"

#ifdef DEBUG_MODE
#define DEBUG
#endif

#include "debug.h"
#include "mode_def.h"
#include "mode_mod.h"
#include "mode_loop_buf.h"

// the table of available modes
const mode_mod_ptr_t ROM_ATTR mode_defs[] = {
  &mode_mod_loop_buf
};

const size_t mode_defs_size = (sizeof(mode_defs) / sizeof(mode_mod_ptr_t));
