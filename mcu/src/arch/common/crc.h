#ifndef CRC_H
#define CRC_H

#include <stdint.h>

extern uint16_t crc16(const uint8_t *data, uint16_t n);
extern uint8_t crc7(const uint8_t* data, uint8_t n);

#endif
