#ifndef PROTO_CMD_SHARED_H
#define PROTO_CMD_SHARED_H

#define PROTO_CMD_RESET           0x01

#define PROTO_CMD_ATTACH          0x40 // action
#define PROTO_CMD_DETACH          0x41 // action
#define PROTO_CMD_GET_STATUS      0x42 // read_word

#define PROTO_CMD_TX_SIZE         0x50 // write_word
#define PROTO_CMD_TX_BUF          0x51 // write_block
#define PROTO_CMD_TX_RESULT       0x52 // read_word

#define PROTO_CMD_RX_SIZE         0x60 // read_word
#define PROTO_CMD_RX_BUF          0x61 // read_block
#define PROTO_CMD_RX_RESULT       0x62 // read_word

#define PROTO_CMD_SET_MODE        0x70 // write_word
#define PROTO_CMD_GET_MODE        0x71 // read_word

#define PROTO_CMD_SET_MAC         0x72 // write_block [6]
#define PROTO_CMD_GET_MAC         0x73 // read_block [6]
#define PROTO_CMD_GET_DEF_MAC     0x74 // read_block [6]

// bitmask of status
#define PROTO_CMD_STATUS_HW_INIT       0x01
#define PROTO_CMD_STATUS_ATTACHED      0x02
#define PROTO_CMD_STATUS_RX_PENDING    0x04

// mode
#define PROTO_CMD_MODE_BRIDGE          0x00
#define PROTO_CMD_MODE_LOOPBACK_BUF    0x01
#define PROTO_CMD_MODE_LOOPBACK_DEV    0x02
#define PROTO_CMD_MODE_MASK            0x03
// mask for transfer type
#define PROTO_CMD_MODE_BUF_TRANSFER    0x00
#define PROTO_CMD_MODE_SPI_TRANSFER    0x80
#define PROTO_CMD_MODE_TRANSFER_MASK   0x80

// result values for RX/TX RESULT
#define PROTO_CMD_RESULT_OK        0
#define PROTO_CMD_RESULT_ERROR     1

#endif
