#include "types.h"

#ifdef DEBUG_MODE
#define DEBUG
#endif

#include "debug.h"
#include "nic.h"
#include "nic_mod.h"
#include "nic_def.h"

// current pointer
nic_mod_ptr_t nic_mod_ptr;
static u08 current_index;

void nic_mod_init(void)
{
  nic_mod_ptr = (nic_mod_ptr_t)read_rom_rom_ptr(nic_defs);
  current_index = 0;
}

u08 nic_mod_get_num_modules(void)
{
  return nic_defs_size;
}

void nic_mod_set_current(u08 index)
{
  if(index < nic_defs_size) {
    nic_mod_ptr = (nic_mod_ptr_t)read_rom_rom_ptr(nic_defs + index);
    current_index = index;
  }
}

u08 nic_mod_get_current(void)
{
  return current_index;
}
