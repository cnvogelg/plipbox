#ifndef LOG_H
#define LOG_H

#include "global.h"

extern void log_init(void);
extern void log_add(u08 code);
extern void log_dump(void);

#endif