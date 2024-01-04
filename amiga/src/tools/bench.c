#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "bench.h"
#include "pktbuf.h"

void bench_loop(bench_data_t *data, bench_opt_t *opt)
{
  ULONG iter;
  sanadev_handle_t *sh = data->sh;
  atimer_handle_t *th = data->th;
  ULONG frame_size = opt->bufsize;
  BOOL ok;

  pktbuf_t *tx_buf;
  pktbuf_t *rx_buf;

  // alloc tx/rx buffer
  tx_buf = pktbuf_alloc(frame_size);
  if(tx_buf == NULL) {
    Printf("No memory for tx buffer!\n");
    return;
  }
  rx_buf = pktbuf_alloc(frame_size);
  if(rx_buf == NULL) {
    Printf("No memory for rx buffer!\n");
    pktbuf_free(tx_buf);
    return;
  }
  // setup signal mask
  ULONG rx_mask = sanadev_io_read_get_mask(sh);
  ULONG timer_mask = atimer_sig_get_mask(th);
  ULONG wait_mask = rx_mask | timer_mask | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D;

  atime_estamp_t sum_eclock = 0;
  ULONG sum_bytes = 0;
  BOOL stay = TRUE;

  // time stamping
  atime_estamp_t time_start;
  atime_estamp_t time_end;

  // prepare buffer
  pktbuf_fill(tx_buf, 0);

  Printf("Bench Loop: Loops=%lu, BufSize=%lu\n", opt->loops, frame_size);
  Printf("EClock Freq: %ld Hz\n", atimer_eclock_freq(th));
  for(iter=0; iter<opt->loops; iter++) {

    Printf("Frame: %ld\n", iter);
    Flush(Output());

    // start timer
    atimer_sig_start(th, opt->timeout, 0);

    // start async receive
    ok = sanadev_io_read_start_orphan(sh, rx_buf->data, rx_buf->size, TRUE);
    if(!ok) {
      sanadev_io_read_print_error(sh);
      break;
    }

    // start time
    atimer_eclock_get(th, &time_start);

    // send sync frame
    ok = sanadev_io_write_raw(sh, tx_buf->data, tx_buf->size);
    if(!ok) {
      sanadev_io_write_print_error(sh);
      break;
    }

    // wait for rx, time out or user abort
    ULONG res_mask = Wait(wait_mask);

    // end time
    atimer_eclock_get(th, &time_end);

    // got result
    if(res_mask & rx_mask) {
      pktbuf_t res_buf;
      ok = sanadev_io_read_result_raw(sh, &res_buf.data, &res_buf.size);
      if(!ok) {
        sanadev_io_read_print_error(sh);
        stay = FALSE;
      }
      else {
        // check buffer
        ULONG pos = 0;
        ok = pktbuf_check(&res_buf, 0, &pos);
        if(!ok) {
          Printf("Rx Buffer mismatch @%ld!\n", pos);
          stay = FALSE;
        } else {
          // calc time
          atime_estamp_t delta;
          atimer_eclock_delta(&time_end, &time_start, &delta);
          sum_eclock += delta;
          sum_bytes  += frame_size * 2; // tx + rx
        }
      }
    }
    // CTRL-C/CTRL-D
    if(res_mask & (SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D)) {
      Printf("Break***\n");
      stay = FALSE;
    }
    // timeout
    if(res_mask & timer_mask) {
      Printf("Time out!\n");
      stay = FALSE;
    }

    atimer_sig_stop(th);
    sanadev_io_read_stop(sh);

    if(!stay) {
      break;
    }
  }

  // timing result
  Printf("Timing Result:\n");
  Printf("Sum EClock: %8ld\n", (ULONG)sum_eclock);
  Printf("Sum Bytes:  %8ld\n", sum_bytes);
  ULONG sum_us = atimer_eclock_to_us(th, &sum_eclock);
  ULONG bps = atimer_eclock_to_bps(th, &sum_eclock, sum_bytes);
  Printf("Sum us:     %8ld\n", sum_us);
  Printf("bytes/s:    %8ld\n", bps);

  // free tx/rx buffer
  pktbuf_free(rx_buf);
  pktbuf_free(tx_buf);
}
