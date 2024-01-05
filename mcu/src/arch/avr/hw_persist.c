#include <avr/eeprom.h>
#include <util/crc16.h>

#include "types.h"
#include "hw_persist.h"

static hw_persist_base_t EEMEM eeprom_data;

// build check sum for parameter block
static uint16_t calc_crc16(const u08 *data, u16 size)
{
  uint16_t crc16 = 0xffff;
  const u08 *ptr = (const u08 *)data;
  for(u16 i=0;i<size;i++) {
    crc16 = _crc16_update(crc16,*ptr);
    ptr++;
  }
  return crc16;
}

u08 hw_persist_save(hw_persist_base_t *base, u16 size)
{
  // check that eeprom is writable
  if(!eeprom_is_ready())
    return HW_PERSIST_NOT_READY;

  // calc check sum
  base->crc = 0;
  // calc current parameter crc
  base->crc = calc_crc16((const u08 *)base, size);

  // write current param to eeprom
  eeprom_update_block(base, &eeprom_data, size);

  return HW_PERSIST_OK;
}

u08 hw_persist_load(hw_persist_base_t *base, u16 size)
{
  // check that eeprom is readable
  if(!eeprom_is_ready())
    return HW_PERSIST_NOT_READY;

  // read param
  eeprom_read_block(base, &eeprom_data, size);

  // get stored checksum
  u16 got_crc = base->crc;

  // recalc
  base->crc = 0;
  uint16_t calc_crc = calc_crc16((const u08 *)base, size);
  if(got_crc != calc_crc) {
    return HW_PERSIST_WRONG_CRC;
  }

  return HW_PERSIST_OK;
}
