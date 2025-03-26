#ifndef MODE_SHARED_H
#define MODE_SHARED_H

#include "tag.h"

// mode values
#define MODE_NIC          0x00  // normal operation as NIC
#define MODE_LOOP_BUF     0x01  // loopback in plipbox buffer (no nic)

// special mode values
#define MODE_FROM_PARAM   0xfe  // use the mode configured by params
#define MODE_NONE         0xfd  // no mode active

// mode_def
// +00 u32 tag
// =04
#define MODE_DEF_SIZE      4

// if an id was not found
#define MODE_ID_INVALID    0xff

// tags
#define MODE_TAG_LOOP     MAKE_TAG('L','O','O','P')
#define MODE_TAG_NIC      MAKE_TAG('N','I','C', 0)

// mode status
#define MODE_STATUS_UNKNOWN    0
#define MODE_STATUS_ATTACHED   1
#define MODE_STATUS_DETACHED   2
#define MODE_STATUS_ERROR_INVALID_MODE        10
#define MODE_STATUS_ERROR_ALREADY_ATTACHED    11
#define MODE_STATUS_ERROR_ALREADY_DETACHED    12
#define MODE_STATUS_ERROR_NIC_INIT            13

#endif
