#ifndef SER_PARSE_H
#define SER_PARSE_H

#include "global.h"

#define SER_PARSE_CMD_OK      0
#define SER_PARSE_CMD_EXIT    1
#define SER_PARSE_CMD_FAIL    2
#define SER_PARSE_CMD_UNKNOWN 3
#define SER_PARSE_CMD_SILENT  4

typedef void (*ser_parse_data_func_t)(u08 data);
typedef u08 (*ser_parse_cmd_func_t)(u08 len, const char *cmd);
typedef void (*ser_parse_func_t)(void);

// init serial parser and set slip data and end functions
extern void ser_parse_set_data_func(ser_parse_data_func_t data_func_t);
extern void ser_parse_set_cmd_func(ser_parse_cmd_func_t cmd_func_t);
extern void ser_parse_set_cmd_enter_leave_func(ser_parse_func_t enter_func, ser_parse_func_t leave_func);

// return true: state change
extern u08 ser_parse_worker(void);

#endif