#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "bench.h"
#include "pktbuf.h"

struct timing {
  atime_estamp_t start;
  atime_estamp_t stop;
  atime_estamp_t delta;
  ULONG us;
  ULONG bytes;
  ULONG bps;
};
typedef struct timing timing_t;

static void timing_calc(atimer_handle_t *th, timing_t *timing)
{
  if(timing->start != timing->stop) {
    atimer_eclock_delta(&timing->stop, &timing->start, &timing->delta);
  }
  timing->us = atimer_eclock_to_us(th, &timing->delta);
  timing->bps = atimer_eclock_to_bps(th, &timing->delta, timing->bytes);
}

static void timing_print(timing_t *timing)
{
  Printf("bytes=%ld, edelta=%ld, us=%ld, bps=%ld\n",
    timing->bytes, (ULONG)timing->delta, timing->us, timing->bps);
}

static void timing_sum(timing_t *t, timing_t *sum)
{
  sum->bytes += t->bytes;
  sum->delta += t->delta;
}

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
  timing_t current;
  timing_t sum = { 0,0,0,0,0,0 };

  // prepare buffer
  pktbuf_fill(tx_buf, 0);

  Printf("Bench Loop: Loops=%lu, BufSize=%lu\n", opt->loops, frame_size);
  Printf("EClock Freq: %ld Hz\n", atimer_eclock_freq(th));
  for(iter=0; iter<opt->loops; iter++) {

    Printf("Frame: %04ld  ", iter);
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
    atimer_eclock_get(th, &current.start);

    // send sync frame
    ok = sanadev_io_write_raw(sh, tx_buf->data, tx_buf->size);
    if(!ok) {
      sanadev_io_write_print_error(sh);
      break;
    }

    // wait for rx, time out or user abort
    ULONG res_mask = Wait(wait_mask);

    // end time
    atimer_eclock_get(th, &current.stop);

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
          // calc and print timing
          current.bytes = 2 * frame_size;
          timing_calc(th, &current);
          timing_print(&current);
          // sum
          timing_sum(&current, &sum);
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
  Printf("Result:  ");
  timing_calc(th, &sum);
  timing_print(&sum);

  // free tx/rx buffer
  pktbuf_free(rx_buf);
  pktbuf_free(tx_buf);
}
