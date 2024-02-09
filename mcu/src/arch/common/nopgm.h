#ifndef NOPGM_H
#define NOPGM_H

typedef const char * rom_pchar;
typedef char *ram_pchar;

#define PSTR(x) x

#define read_rom_char(x)       (*((const unsigned char *)(x)))
#define read_rom_word(x)       (*((const unsigned short *)(x)))
#define read_rom_long(x)       (*((const unsigned long *)(x)))

#define read_rom_rom_ptr(x)    (*(x))
#define read_rom_ram_ptr(x)    (*(x))

#define ROM_ATTR

static inline void rom_copy(const unsigned char *src, unsigned char *dst, size_t size)
{
  for(size_t i=0;i<size;i++) {
    *(dst++) = *(src++);
  }
}

#endif
