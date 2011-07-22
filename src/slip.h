#ifndef SLIP_H
#define SLIP_H

#include "global.h"

#define SLIP_STATUS_ERROR    0
#define SLIP_STATUS_OK       1
#define SLIP_STATUS_END      2

// read: result: 0=error 1=ok 2=END
extern u08 slip_read(u08 *data);
// send: result: 0=error 1=ok
extern u08 slip_send(u08 data);
extern u08 slip_send_end(void);

#endif
