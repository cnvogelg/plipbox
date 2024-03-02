#ifndef PROTO_CMD_SHARED_H
#define PROTO_CMD_SHARED_H

// ----- proto command bytes -----

#define PROTO_CMD_INIT            0x20 // write_word
#define PROTO_CMD_PING            0x21 // read_word
#define PROTO_CMD_EXIT            0x22 // action

#define PROTO_CMD_GET_VERSION     0x30 // read_word
#define PROTO_CMD_GET_CUR_MAC     0x31 // read_block [6]
#define PROTO_CMD_GET_DEF_MAC     0x32 // read block [6]
#define PROTO_CMD_SET_CUR_MAC     0x33 // write_block [6]

#define PROTO_CMD_ATTACH          0x40 // action
#define PROTO_CMD_DETACH          0x41 // action
#define PROTO_CMD_GET_STATUS      0x42 // read_word

#define PROTO_CMD_TX_SIZE         0x50 // write_word
#define PROTO_CMD_TX_BUF          0x51 // write_block
#define PROTO_CMD_TX_RESULT       0x52 // read_word - return status + tx result

#define PROTO_CMD_RX_SIZE         0x60 // read_word
#define PROTO_CMD_RX_BUF          0x61 // read_block
#define PROTO_CMD_RX_RESULT       0x62 // read_word - return status + rx result

// param commands
#define PROTO_CMD_PARAM_GET_NUM   0x70 // read_word
#define PROTO_CMD_PARAM_FIND_TAG  0x71 // write_long
#define PROTO_CMD_PARAM_SET_ID    0x72 // write word
#define PROTO_CMD_PARAM_GET_ID    0x73 // read_word
#define PROTO_CMD_PARAM_GET_DEF   0x74 // read_block [PARAM_DEF_SIZE]
#define PROTO_CMD_PARAM_GET_VAL   0x75 // read_block [param.size]
#define PROTO_CMD_PARAM_SET_VAL   0x76 // write_block [param.size]

// prefs commands
#define PROTO_CMD_PREFS_RESET     0x78 // action
#define PROTO_CMD_PREFS_LOAD      0x79 // read_word [status]
#define PROTO_CMD_PREFS_SAVE      0x7a // read_word [status]

// ----- bits for options -----

// bitmask of status
#define PROTO_CMD_STATUS_IDLE          0x00
#define PROTO_CMD_STATUS_ATTACHED      0x01
#define PROTO_CMD_STATUS_RX_PENDING    0x02
#define PROTO_CMD_STATUS_LINK_UP       0x04

#define PROTO_CMD_STATUS_INIT_ERROR    0x20  // if attach fails
#define PROTO_CMD_STATUS_RX_ERROR      0x40  // only set with RX_RESULT
#define PROTO_CMD_STATUS_TX_ERROR      0x80  // only set with TX_RESULT

#endif
