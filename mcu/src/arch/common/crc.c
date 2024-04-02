#include "crc.h"

static uint16_t crc_xmodem_update(uint16_t crc, uint8_t data)
{
  int i;

  crc = crc ^ ((uint16_t)data << 8);
  for (i=0; i<8; i++)
  {
    if (crc & 0x8000) {
      crc = (crc << 1) ^ 0x1021;
    } else {
      crc <<= 1;
    }
  }
  return crc;
}

uint16_t crc16(const uint8_t *data, uint16_t n)
{
  uint16_t crc16 = 0xffff;
  const uint8_t *ptr = (const uint8_t *)data;
  for(uint16_t i=0;i<n;i++) {
    crc16 = crc_xmodem_update(crc16,*ptr);
    ptr++;
  }
  return crc16;
}

uint8_t crc7(const uint8_t* data, uint8_t n)
{
  uint8_t crc = 0;
  for (uint8_t i = 0; i < n; i++) {
    uint8_t d = data[i];
    for (uint8_t j = 0; j < 8; j++) {
      crc <<= 1;
      if ((d & 0x80) ^ (crc & 0x80)) {
        crc ^= 0x09;
      }
      d <<= 1;
    }
  }
  return (crc << 1) | 1;
}
