#ifndef BENCH_H
#define BENCH_H

#include "sanadev.h"
#include "atimer.h"

struct bench_data
{
  sanadev_handle_t *sh;
  atimer_handle_t  *th;
  APTR              frame;
};
typedef struct bench_data bench_data_t;

struct bench_opt
{
  ULONG loops; // 0=infinite
  ULONG delay; // 0=no delay, n=Delay(n)
  ULONG timeout; // in secs
  ULONG bufsize; // in bytes
};
typedef struct bench_opt bench_opt_t;

extern void bench_loop(bench_data_t *data, bench_opt_t *opt);

#endif
