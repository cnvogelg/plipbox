#ifndef PARAM_SHARED_H
#define PARAM_SHARED_H

#include "types.h"

#define MAKE_TAG(a,b,c,d)      ( (((u32)a)<<24) | (((u32)b)<<16) | (((u32)c)<<8) | ((u32)d) )

// tags
#define PARAM_TAG_MAC_ADDR      MAKE_TAG('M','A','C',0)
#define PARAM_TAG_MODE          MAKE_TAG('M','O','D','E')
#define PARAM_TAG_NIC           MAKE_TAG('N','I','C',0)
#define PARAM_TAG_NCAP          MAKE_TAG('N','C','A','P')
#define PARAM_TAG_NPRT          MAKE_TAG('N','P','R','T')

// param description
// +00 u08 index
// +01 u08 type
// +02 u08 format
// +03 u08 reserved
// +04 u16 size (must be even!!)
// +06 u32 tag
// =10
#define PARAM_DEF_SIZE      10

// if an id was not found
#define PARAM_ID_INVALID    0xff

// param type
#define PARAM_TYPE_WORD        1
#define PARAM_TYPE_LONG        2
#define PARAM_TYPE_BYTE_ARRAY  3
#define PARAM_TYPE_WORD_ARRAY  4
#define PARAM_TYPE_LONG_ARRAY  5

// size
#define PARAM_SIZE_WORD   2
#define PARAM_SIZE_LONG   4

// param format
#define PARAM_FORMAT_DEC  0 // default decimal
#define PARAM_FORMAT_HEX  1 // hex data, like MACs
#define PARAM_FORMAT_BIN  2 // binary data for flags
#define PARAM_FORMAT_STR  4 // string, like SSID or password

#endif
