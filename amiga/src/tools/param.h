#ifndef PARAM_H
#define PARAM_H

#include <devices/plipbox.h>

#define PARAM_OK 0
#define PARAM_WRONG_TAG_SIZE 1
#define PARAM_NO_DIGIT_CHAR 2
#define PARAM_DIGIT_NOT_IN_BASE 3
#define PARAM_WRONG_DATA_SIZE 4
#define PARAM_WRONG_TYPE 5
#define PARAM_DATA_TOO_LONG 6
#define PARAM_DATA_TOO_SHORT 7
#define PARAM_WRONG_BASE 8

extern int param_parse_tag(const char *str, ULONG *tag);
extern int param_parse_val(const char *str, s2pb_param_def_t *def, UBYTE *data);
extern int param_print_val(char *str, s2pb_param_def_t *def, const UBYTE *data);
extern char *param_perror(int res);

extern void param_set_wire_value(UBYTE *data, ULONG number, int value_bytes);
extern ULONG param_get_wire_value(const UBYTE *data, int value_bytes);

#endif
