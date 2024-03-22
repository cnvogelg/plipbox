#ifndef PARAM_H
#define PARAM_H

#include "param_shared.h"

struct param_def {
  u08   index;
  u08   type;
  u08   format;
  u16   size;
  u32   tag;
};
typedef struct param_def param_def_t;

#define PARAM_PARSE_OK 0
#define PARAM_PARSE_WRONG_TAG_SIZE 1
#define PARAM_PARSE_NO_DIGIT_CHAR 2
#define PARAM_PARSE_DIGIT_NOT_IN_BASE 3
#define PARAM_PARSE_WRONG_DATA_SIZE 4
#define PARAM_PARSE_WRONG_TYPE 5
#define PARAM_PARSE_DATA_TOO_LONG 6
#define PARAM_PARSE_DATA_TOO_SHORT 7
#define PARAM_PARSE_WRONG_BASE 8

extern int param_parse_tag(const char *str, ULONG *tag);
extern int param_parse_val(const char *str, param_def_t *def, UBYTE *data);
extern int param_print_val(char *str, param_def_t *def, const UBYTE *data);
extern char *param_parse_perror(int res);

extern void param_set_wire_value(UBYTE *data, ULONG number, int value_bytes);
extern ULONG param_get_wire_value(const UBYTE *data, int value_bytes);

#endif
