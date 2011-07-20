#ifndef PLIP_H
#define PLIP_H

#include "global.h"

#define PLIP_STATUS_OK              0
#define PLIP_STATUS_NO_MAGIC        1
#define PLIP_STATUS_INVALID_MAGIC   2
#define PLIP_STATUS_RX_TIMEOUT      3

#define PLIP_MAGIC        0x42
#define PLIP_NOCRC        0x02
#define PLIP_CRC          0x01

typedef struct {
  u08 crc_type;
  u16 size;
  u16 crc;
  u32 type;
} plip_packet_t;

typedef u08 (*plip_begin_rx_frame_func)(plip_packet_t *pkt);
typedef u08 (*plip_fill_rx_frame_func)(u08 data);

// ----- Parameter -----

extern u16 plip_rx_timeout; // timeout for next byte in 100us

// ----- Init -----

extern void plip_init(plip_begin_rx_frame_func begin_func, plip_fill_rx_frame_func fill_func);

// ----- Recv -----

extern u08 plip_is_recv_begin(void);
extern u08 plip_recv(plip_packet_t *pkt);

#endif
