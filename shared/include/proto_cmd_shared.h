#ifndef PROTO_CMD_SHARED_H
#define PROTO_CMD_SHARED_H

// ----- proto command bytes -----

#define PROTO_CMD_RESET           0x10 // action
#define PROTO_CMD_ALIVE           0x11 // action_wo_busy

#define PROTO_CMD_INIT            0x20 // write_word
#define PROTO_CMD_PING            0x21 // read_word
#define PROTO_CMD_EXIT            0x22 // action
#define PROTO_CMD_GET_VERSION     0x23 // read_word

#define PROTO_CMD_ATTACH          0x40 // action
#define PROTO_CMD_DETACH          0x41 // action
#define PROTO_CMD_EVENT_MASK      0x42 // read_word
#define PROTO_CMD_LINK_STATUS     0x43 // read_word
#define PROTO_CMD_HW_STATUS       0x44 // read_word

#define PROTO_CMD_TX_SIZE         0x50 // write_word
#define PROTO_CMD_TX_BUF          0x51 // write_block
#define PROTO_CMD_TX_RESULT       0x52 // read_word - return status + tx result
#define PROTO_CMD_TX_ERROR        0x53 // read_word
#define PROTO_CMD_TX_DROP_COUNT   0x54 // read_word

#define PROTO_CMD_RX_SIZE         0x60 // read_word
#define PROTO_CMD_RX_BUF          0x61 // read_block
#define PROTO_CMD_RX_RESULT       0x62 // read_word - return status + rx result
#define PROTO_CMD_RX_ERROR        0x63 // read_word
#define PROTO_CMD_RX_DROP_COUNT   0x64 // read_word

// custom requests
#define PROTO_CMD_REQ_IN          0x70 // write_long (oper_id, in_size)
#define PROTO_CMD_REQ_IN_DATA     0x71 // write_block (in_size)
#define PROTO_CMD_REQ_OUT         0x72 // read_long (status, out_size)
#define PROTO_CMD_REQ_OUT_DATA    0x73 // read_block (out_size)
#define PROTO_CMD_REQ_EVENT_MASK  0x74 // read_word

#endif
