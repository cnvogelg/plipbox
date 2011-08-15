#ifndef SLIP_H
#define SLIP_H

#include "global.h"

#define SLIP_STATUS_ERROR    0
#define SLIP_STATUS_OK       1
#define SLIP_STATUS_END      2

typedef void (*slip_end_func_t)(void);
typedef void (*slip_data_func_t)(u08 data);

// read with callback
extern void slip_push_init(slip_data_func_t data_func, slip_end_func_t end_func);
extern void slip_push(u08 data);

// read: result: 0=error 1=ok 2=END
extern u08 slip_read(u08 *data);
// send: result: 0=error 1=ok
extern u08 slip_send(u08 data);
extern u08 slip_send_end(void);

#endif
