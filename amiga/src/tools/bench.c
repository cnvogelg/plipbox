#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "bench.h"

void bench_loop(sanadev_handle_t *sh, bench_opt_t *opt)
{
  Printf("Bench Loop: Loops=%lu\n", opt->loops);
}
