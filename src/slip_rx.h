#ifndef SLIP_RX_H
#define SLIP_RX_H

#include "global.h"

extern void slip_rx_init(void);
extern void slip_rx_data(u08 data);
extern void slip_rx_end(void);
extern u08 slip_rx_worker(void);

#endif
