#ifndef PROTO_CMD_SHARED_H
#define PROTO_CMD_SHARED_H

// ----- proto command bytes -----

#define PROTO_CMD_INIT            0x20 // write_word
#define PROTO_CMD_PING            0x21 // read_word
#define PROTO_CMD_EXIT            0x22 // action
#define PROTO_CMD_GET_VERSION     0x23 // read_word

#define PROTO_CMD_ATTACH          0x40 // action
#define PROTO_CMD_DETACH          0x41 // action
#define PROTO_CMD_GET_STATUS      0x42 // read_word

#define PROTO_CMD_TX_SIZE         0x50 // write_word
#define PROTO_CMD_TX_BUF          0x51 // write_block
#define PROTO_CMD_TX_RESULT       0x52 // read_word - return status + tx result

#define PROTO_CMD_RX_SIZE         0x60 // read_word
#define PROTO_CMD_RX_BUF          0x61 // read_block
#define PROTO_CMD_RX_RESULT       0x62 // read_word - return status + rx result

// custom requests
#define PROTO_CMD_REQ_IN          0x70 // write_long (oper_id, in_size)
#define PROTO_CMD_REQ_IN_DATA     0x71 // write_block (in_size)
#define PROTO_CMD_REQ_OUT         0x72 // read_long (status, out_size)
#define PROTO_CMD_REQ_OUT_DATA    0x73 // read_block (out_size)
#define PROTO_CMD_REQ_EVENTS      0x74 // read_word

// ----- bits for options -----

// bitmask of status
#define PROTO_CMD_STATUS_IDLE          0x00
#define PROTO_CMD_STATUS_ATTACHED      0x01
#define PROTO_CMD_STATUS_RX_PENDING    0x02
#define PROTO_CMD_STATUS_LINK_UP       0x04

#define PROTO_CMD_STATUS_REQ_EVENTS    0x10
#define PROTO_CMD_STATUS_INIT_ERROR    0x20  // if attach fails
#define PROTO_CMD_STATUS_RX_ERROR      0x40  // only set with RX_RESULT
#define PROTO_CMD_STATUS_TX_ERROR      0x80  // only set with TX_RESULT

#endif
