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

static inline void wire_w2h_u16(const u08 *in, u16 *w)
{
  *w = (u16)in[0] << 8 | (u16)in[1];
}

static inline void wire_w2h_u32(const u08 *in, u32 *w)
{
  *w = (u32)in[0] << 24 | (u32)in[1] << 16 | (u32)in[2] << 8 | (u32)in[3];
}

#endif
