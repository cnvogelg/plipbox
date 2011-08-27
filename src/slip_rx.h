#ifndef SLIP_RX_H
#define SLIP_RX_H

#include "global.h"

#define SLIP_RX_RESULT_IDLE      0
#define SLIP_RX_RESULT_TX_OK     1
#define SLIP_RX_RESULT_TX_RETRY  2
#define SLIP_RX_RESULT_TX_FAIL   3
#define SLIP_RX_RESULT_DROP      4
#define SLIP_RX_RESULT_PLIP_RX_BEGUN 5
#define SLIP_RX_RESULT_PLIP_RX_BEGUN_SKIP 6 

extern void slip_rx_init(void);
extern u08 slip_rx_worker(void);

#endif
