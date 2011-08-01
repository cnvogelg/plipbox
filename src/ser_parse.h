#ifndef SER_PARSE_H
#define SER_PARSE_H

#include "global.h"

#define SER_PARSE_CMD_OK    0
#define SER_PARSE_CMD_EXIT  1
#define SER_PARSE_CMD_FAIL  2

typedef void (*ser_parse_data_func_t)(u08 data);
typedef u08 (*ser_parse_cmd_func_t)(u08 len, const char *cmd);

// init serial parser and set slip data and end functions
extern void ser_parse_init(ser_parse_data_func_t data_func_t,
                           ser_parse_cmd_func_t cmd_func_t);

// return true: state change
extern u08 ser_parse_worker(void);

#endif