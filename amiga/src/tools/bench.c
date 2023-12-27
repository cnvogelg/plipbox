#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "bench.h"

void bench_loop(bench_data_t *data, bench_opt_t *opt)
{
  ULONG iter;
  sanadev_handle_t *sh = data->sh;
  atimer_handle_t *th = data->th;
  APTR frame = data->frame;
  ULONG frame_size = opt->bufsize;

  Printf("Bench Loop: Loops=%lu, BufSize=%lu\n", opt->loops, frame_size);
  for(iter=0; iter<opt->loops; iter++) {

    // send frame
    Printf("Frame: %ld\n", iter);
    BOOL ok = sanadev_io_write_raw(sh, frame, frame_size);
    if(!ok) {
      sanadev_io_write_print_error(sh);
      break;
    }


  }
}
