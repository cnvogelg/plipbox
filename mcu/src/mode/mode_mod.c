#include "types.h"

#ifdef DEBUG_MODE
#define DEBUG
#endif

#include "debug.h"
#include "mode.h"
#include "mode_mod.h"
#include "mode_def.h"

// current pointer
mode_mod_ptr_t mode_mod_ptr;
static u08 current_index;

void mode_mod_init(void)
{
  mode_mod_ptr = (mode_mod_ptr_t)read_rom_rom_ptr(mode_defs);
  current_index = 0;
}

u08 mode_mod_get_num_modes(void)
{
  return mode_defs_size;
}

void mode_mod_set_current(u08 index)
{
  if(index < mode_defs_size) {
    mode_mod_ptr = (mode_mod_ptr_t)read_rom_rom_ptr(mode_defs + index);
    current_index = index;
  }
}

u08 mode_mod_get_current(void)
{
  return current_index;
}

u32 mode_mod_tag_at(u08 index)
{
  mode_mod_ptr_t pd = (mode_mod_ptr_t)read_rom_rom_ptr(mode_defs + index);
  return read_rom_long(&pd->tag);
}

void mode_mod_def_at(u08 index, mode_def_t *def)
{
  mode_mod_ptr_t pd = (mode_mod_ptr_t)read_rom_rom_ptr(mode_defs + index);
  def->tag = read_rom_long(&pd->tag);
}
