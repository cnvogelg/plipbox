#include "types.h"

#ifdef DEBUG_MODE
#define DEBUG
#endif

#include "debug.h"
#include "mode.h"
#include "mode_mod.h"

#include "loop_buf.h"

// the table of available modes
const mode_mod_ptr_t ROM_ATTR mode_map[] = {
  &mode_mod_loop_buf
};
#define NUM_MODES  (sizeof(mode_map) / sizeof(mode_mod_ptr_t))


// current pointer
mode_mod_ptr_t mode_mod_ptr;
static u08 current_index;

void mode_mod_init(void)
{
  mode_mod_ptr = (mode_mod_ptr_t)read_rom_rom_ptr(mode_map);
  current_index = 0;
}

u08 mode_mod_get_num_modules(void)
{
  return NUM_MODES;
}

void mode_mod_set_current(u08 index)
{
  if(index < NUM_MODES) {
    mode_mod_ptr = (mode_mod_ptr_t)read_rom_rom_ptr(mode_map + index);
    current_index = index;
  }
}

u08 mode_mod_get_current(void)
{
  return current_index;
}
