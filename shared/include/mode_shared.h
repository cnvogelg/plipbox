#ifndef MODE_SHARED_H
#define MODE_SHARED_H

#include "tag.h"

// mode values
#define MODE_NIC          0x00  // normal operation as NIC
#define MODE_LOOP_BUF     0x01  // loopback in plipbox buffer (no nic)

// mode_def
// +00 u32 tag
// =04
#define MODE_DEF_SIZE      4

// if an id was not found
#define MODE_ID_INVALID    0xff

// tags
#define MODE_TAG_LOOP     MAKE_TAG('L','O','O','P')
#define MODE_TAG_NIC      MAKE_TAG('N','I','C', 0)

#endif
