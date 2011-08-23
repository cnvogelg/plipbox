#ifndef BENCH_H
#define BENCH_H

#include "global.h"

extern void bench_begin(void);
extern void bench_end(void);
extern void bench_submit(u16 bytes);
extern void bench_dump(void);

#endif
