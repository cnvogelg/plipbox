#ifndef PARAM_SHARED_H
#define PARAM_SHARED_H

// tags
#define PARAM_TAG_MAC_ADDR      0x4d414300  // MAC\0
#define PARAM_TAG_MODE          0x4d4f4445  // MODE
#define PARAM_TAG_NIC           0x4e494300  // NIC
#define PARAM_TAG_FLAG          0x464c4147  // FLAG
#define PARAM_TAG_DLY           0x444c5900  // DLY
#define PARAM_TAG_IP            0x49500000  // IP
#define PARAM_TAG_NMSK          0x4e4d534b  // NMSK

// mode values
#define PARAM_MODE_NIC            0x00  // normal operation as NIC
#define PARAM_MODE_LOOPBACK_BUF   0x01  // loopback in plipbox buffer (no pio)
#define PARAM_MODE_LOOPBACK_DEV   0x02  // loopback in PIO's buffer
#define PARAM_MODE_LOOPBACK_MAC   0x03  // loopback in PIO's MAC

// flag values
#define PARAM_FLAG_PIO_FULL_DUPLEX    0x01
#define PARAM_FLAG_PIO_FLOW_CONTROL   0x02
#define PARAM_FLAG_PIO_DIRECT_SPI     0x04  // transfer buffer directly to pio

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
