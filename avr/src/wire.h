#ifndef WIRE_H
#define WIRE_H

// big endian

static inline void wire_h2w_u16(u16 w, u08 *out)
{
  out[0] = (u08)((w >> 8) & 0xff);
  out[1] = (u08)(w & 0xff);
}

static inline void wire_h2w_u32(u32 w, u08 *out)
{
  out[0] = (u08)((w >> 24) & 0xff);
  out[1] = (u08)((w >> 16) & 0xff);
  out[2] = (u08)((w >> 8) & 0xff);
  out[3] = (u08)(w & 0xff);
}

#endif
