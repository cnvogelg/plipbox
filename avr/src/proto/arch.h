#ifndef ARCH_H
#define ARCH_H

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

typedef PGM_P rom_pchar;
typedef char *ram_pchar;

#define read_rom_char(x)       pgm_read_byte_near(x)
#define read_rom_word(x)       pgm_read_word_near(x)
#define read_rom_long(x)       pgm_read_dword_near(x)

#define read_rom_rom_ptr(x)    ((rom_pchar)pgm_read_word_near(x))
#define read_rom_ram_ptr(x)    ((ram_pchar)pgm_read_word_near(x))

static inline void rom_copy(const unsigned char *src, unsigned char *dst, size_t size)
{
  for(size_t i=0;i<size;i++) {
    *(dst++) = pgm_read_byte(src++);
  }
}

#define ROM_ATTR      __ATTR_PROGMEM__
#define EEPROM_ATTR   EEMEM

#define INLINE          static inline
#define FORCE_INLINE    __attribute__((always_inline)) static inline

typedef uint16_t flash_size_t;

#endif
