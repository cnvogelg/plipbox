#include <string.h>

#include "hardware/flash.h"
#include "hardware/regs/addressmap.h"
#include "hardware/sync.h"

#include "types.h"
#include "hw_persist.h"
#include "crc.h"

#define PREF_SIZE   FLASH_SECTOR_SIZE

const char* PREF_BASE = (char*)(PICO_FLASH_SIZE_BYTES - PREF_SIZE);

u08 hw_persist_save(hw_persist_base_t *base, u16 size)
{
  uint32_t p = (uint32_t)PREF_BASE;

  base->crc = 0;
  base->crc = crc16((const u08 *)base, size);

  uint32_t ints = save_and_disable_interrupts();
  flash_range_erase(p, PREF_SIZE);
  flash_range_program(p, (const u08 *)base, PREF_SIZE);
  restore_interrupts(ints);

  return HW_PERSIST_OK;
}

u08 hw_persist_load(hw_persist_base_t *base, u16 size)
{
  memcpy(base, PREF_BASE + XIP_NOCACHE_NOALLOC_BASE, size);

  // get stored checksum
  u16 got_crc = base->crc;

  // recalc
  base->crc = 0;
  uint16_t calc_crc = crc16((const u08 *)base, size);
  if(got_crc != calc_crc) {
    return HW_PERSIST_WRONG_CRC;
  }

  return HW_PERSIST_OK;
}
