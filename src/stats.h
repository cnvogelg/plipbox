#ifndef STATS_H
#define STATS_H

#include "global.h"

typedef struct {
  u16 pkt_rx_cnt;
  u16 pkt_tx_cnt;
  u16 pkt_tx_err;
  u16 pkt_rx_err;
  u32 pkt_rx_bytes;
  u32 pkt_tx_bytes;
  u08 pkt_last_tx_err;
  u08 pkt_last_rx_err;

  u16 pkt_time;
} stats_t;

extern stats_t stats;

extern void stats_reset(void);

extern void stats_reset_timing(void);
extern void stats_capture_timing(void);

extern void stats_dump(void);

#endif
