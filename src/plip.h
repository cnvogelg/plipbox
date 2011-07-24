#ifndef PLIP_H
#define PLIP_H

#include "global.h"

#define PLIP_STATUS_OK              0
#define PLIP_STATUS_NO_MAGIC        1
#define PLIP_STATUS_INVALID_MAGIC   2
#define PLIP_STATUS_TIMEOUT         3
#define PLIP_STATUS_INVALID_START   5
#define PLIP_STATUS_CALLBACK_FAILED 6

#define PLIP_STATE_MAGIC            0x10
#define PLIP_STATE_CRC_TYPE         0x20
#define PLIP_STATE_SIZE             0x30
#define PLIP_STATE_CRC              0x40
#define PLIP_STATE_TYPE             0x50
#define PLIP_STATE_DATA             0x60
#define PLIP_STATE_LAST_DATA        0x70

#define PLIP_MAGIC        0x42
#define PLIP_NOCRC        0x02
#define PLIP_CRC          0x01

typedef struct {
  u08 crc_type;
  u16 size;
  u16 crc;
  u32 type;
  u16 real_size; // will be set after tx/rx
} plip_packet_t;

typedef u08 (*plip_packet_func)(plip_packet_t *pkt);
typedef u08 (*plip_data_func)(u08 *data);

// ----- Parameter -----

extern u16 plip_rx_timeout; // timeout for next byte in 100us

// ----- Init -----

extern void plip_recv_init(plip_packet_func rx_begin_func, 
                           plip_data_func rx_fill_func,
                           plip_packet_func rx_end_func);

extern void plip_send_init(plip_data_func tx_fill_func);

// ----- Recv -----

extern u08 plip_is_recv_begin(void);
extern u08 plip_recv(plip_packet_t *pkt);

// ----- Send -----

extern u08 plip_is_send_allowed(void);
extern u08 plip_send(plip_packet_t *pkt);

#endif
