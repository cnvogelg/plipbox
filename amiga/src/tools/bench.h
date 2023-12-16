#ifndef BENCH_H
#define BENCH_H

#include "sanadev.h"

struct bench_opt {
  ULONG loops; // 0=infinite
};
typedef struct bench_opt bench_opt_t;

extern void bench_loop(sanadev_handle_t *sh, bench_opt_t *opt);

#endif
