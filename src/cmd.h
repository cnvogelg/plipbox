#ifndef CMD_H
#define CMD_H

#include "global.h"

#define CMD_OK      0
#define CMD_UNKNOWN 1
#define CMD_EXIT    2

extern u08 cmd_parse(u08 len, const char *cmd);

#endif
