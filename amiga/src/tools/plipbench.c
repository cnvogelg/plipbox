#include <exec/exec.h>
#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include "sanadev.h"
#include "param_tag.h"
#include "param_shared.h"
#include "nic_shared.h"
#include "mode_shared.h"
#include "bench.h"
#include "atimer.h"

#define LOG(x)          \
  do                    \
  {                     \
    if (params.verbose) \
    {                   \
      Printf x;         \
      Flush(Output());  \
    }                   \
  } while (0)

/* lib bases */
extern struct ExecBase *SysBase;
#ifdef __SASC
extern struct DosLibrary *DOSBase;
#else
struct DosLibrary *DOSBase;
#endif

/* params */
static const char *TEMPLATE =
    "-D=DEVICE/K,"
    "-U=UNIT/N/K,"
    "-V=VERBOSE/S,"
    "LOOPBACK_BUF/S,"
    "LOOPBACK_INT/S,"
    "LOOPBACK_EXT/S,"
    "FULL_DUPLEX/S,"
    "DIRECT_SPI/S,"
    "LOOPS/N/K,"
    "DELAY/N/K,"
    "TIMEOUT/N/K,"
    "BUFSIZE/N/K,"
    "TIMING/S";
typedef struct
{
  char *device;
  ULONG *unit;
  ULONG verbose;
  ULONG loopback_buf;
  ULONG loopback_int;
  ULONG loopback_ext;
  ULONG full_duplex;
  ULONG direct_spi;
  ULONG *loops;
  ULONG *delay;
  ULONG *timeout;
  ULONG *bufsize;
  ULONG timing;
} params_t;
static params_t params;

static UWORD old_mode;
static UWORD old_ncap;

static BOOL set_mode_and_flags(sanadev_handle_t *sh)
{
  int res;
  UWORD ncap = 0;
  UWORD mode = 0;

  // save old mode
  res = param_tag_mode_get(sh, &old_mode);
  if (res != REQ_OK)
  {
    return FALSE;
  }
  LOG(("Old Mode: %ld\n", (LONG)old_mode));

  // set new mode
  if (params.loopback_buf)
  {
    LOG(("Mode: loopback with plipbox buffer\n"));
    mode = MODE_LOOP_BUF;
  }
  else if (params.loopback_int)
  {
    LOG(("Mode: loopback internal with NIC\n"));
    mode = MODE_NIC;
    ncap = NIC_CAP_LOOP_BACK;
  }
  else if (params.loopback_ext)
  {
    LOG(("Mode: loopback external with loopback cablel\n"));
    mode = MODE_NIC;
    ncap = NIC_CAP_FULL_DUPLEX;
  }
  LOG(("Setting mode: %ld\n", (LONG)mode));
  res = param_tag_mode_set(sh, mode);
  if (res != REQ_OK)
  {
    return FALSE;
  }

  // save old nic caps
  res = param_tag_ncap_get(sh, &old_ncap);
  if (res != REQ_OK)
  {
    return FALSE;
  }
  LOG(("Old NIC Caps: $%lx\n", (LONG)old_ncap));

  // full duplex
  if (params.full_duplex)
  {
    LOG(("NIC Cap: Full Duplex\n"));
    ncap |= NIC_CAP_FULL_DUPLEX;
  }

  // set new flag
  if (params.direct_spi)
  {
    LOG(("NIC Cap: direct SPI transfer to NIC\n"));
    ncap |= NIC_CAP_DIRECT_IO;
  }
  LOG(("Setting NIC Caps: $%lx\n", (LONG)ncap));
  res = param_tag_ncap_set(sh, ncap);
  return (res == REQ_OK);
}

static BOOL restore_mode_and_flags(sanadev_handle_t *sh)
{
  int res;

  LOG(("Restore Mode: %ld\n", (LONG)old_mode));
  res = param_tag_mode_set(sh, old_mode);
  if (res != REQ_OK)
  {
    return FALSE;
  }

  LOG(("Restore NIC Caps: %ld\n", (LONG)old_ncap));
  res = param_tag_ncap_set(sh, old_ncap);
  return (res == REQ_OK) ? TRUE : FALSE;
}

static void setup_bench_opt(bench_opt_t *opt)
{
  if (params.loops != NULL)
  {
    opt->loops = *params.loops;
  }
  else
  {
    opt->loops = 10;
  }
  LOG(("Bench Loops: %ld\n", opt->loops));

  if (params.delay != NULL)
  {
    opt->delay = *params.delay;
  }
  else
  {
    opt->delay = 0;
  }
  LOG(("Iter Delay: %ld\n", opt->delay));

  if (params.timeout != NULL)
  {
    opt->timeout = *params.timeout;
  }
  else
  {
    opt->timeout = 5;
  }
  LOG(("Timeout: %ld\n", opt->timeout));

  if (params.bufsize != NULL)
  {
    opt->bufsize = *params.bufsize;
    if(opt->bufsize < SANADEV_ETH_MIN_FRAME_SIZE) {
      opt->bufsize = SANADEV_ETH_MIN_FRAME_SIZE;
    }
    else if(opt->bufsize > SANADEV_ETH_MAX_FRAME_SIZE) {
      opt->bufsize = SANADEV_ETH_MAX_FRAME_SIZE;
    }
  }
  else
  {
    opt->bufsize = SANADEV_ETH_RAW_FRAME_SIZE;
  }
  LOG(("Bufsize: %ld\n", opt->bufsize));

  if (params.timing != 0)
  {
    opt->timing = TRUE;
  } else {
    opt->timing = FALSE;
  }
  LOG(("Timing: %ld\n", (ULONG)opt->timing));
}

/* ----- tool ----- */
static int plipbench(const char *device, LONG unit)
{
  UWORD error;
  BOOL ok;
  bench_data_t data;
  bench_opt_t bench_opt;

  // parse options
  setup_bench_opt(&bench_opt);

  // setup timer
  LOG(("Opening timer!\n"));
  data.th = atimer_init((struct Library *)SysBase);
  if (data.th == NULL)
  {
    FreeVec(data.frame);
    PutStr("Error: no timer!\n");
    return RETURN_ERROR;
  }
  ok = atimer_sig_init(data.th);
  if (!ok)
  {
    PutStr("Error: no timer signal!\n");
    FreeVec(data.frame);
    atimer_exit(data.th);
    return RETURN_ERROR;
  }

  // enable timing in driver
  ULONG flags = 0;
  if(bench_opt.timing) {
    flags = S2PB_OPF_REQ_TIMING;
  }

  // open plipbox.device
  LOG(("Opening device '%s' unit #%ld with flags %lx\n", device, unit, flags));
  data.sh = sanadev_open(device, unit, flags, &error);
  if (data.sh == NULL)
  {
    Printf("Error opening device '%s' unit #%ld: code=%ld\n", device, unit, (LONG)error);
    FreeVec(data.frame);
    atimer_exit(data.th);
    return RETURN_ERROR;
  }

  // set mode and flag
  ok = set_mode_and_flags(data.sh);
  if (ok)
  {

    // setup io
    ok = sanadev_io_init(data.sh, &error);
    if (!ok)
    {
      Printf("Error setting up SANA-II I/O! code=%ld\n", (LONG)error);
    }
    else
    {
      // go online
      PutStr("going online...\n");
      Flush(Output());
      ok = sanadev_cmd_online(data.sh);
      if (!ok)
      {
        PutStr("Error going online!\n");
        sanadev_cmd_print_error(data.sh);
      }
      else
      {
        // we made it: enter main loop
        bench_loop(&data, &bench_opt);

        // finally go offline
        PutStr("going offline...\n");
        ok = sanadev_cmd_offline(data.sh);
        if (!ok)
        {
          PutStr("Error going offline!\n");
          sanadev_cmd_print_error(data.sh);
        }
      }

      LOG(("Sana I/O exit\n"));
      sanadev_io_exit(data.sh);
    }

    // return to old mode
    restore_mode_and_flags(data.sh);
  }
  else
  {
    PutStr("Error setting mode and flag and device...\n");
  }

  LOG(("Closing device...\n"));
  sanadev_close(data.sh);
  LOG(("Done\n"));

  LOG(("Freeing timer\n"));
  atimer_sig_exit(data.th);
  atimer_exit(data.th);

  return RETURN_OK;
}

/* ---------- main ---------- */
int main(void)
{
  struct RDArgs *args;
  char *device = "plipbox.device";
  LONG unit = 0;
  int result;

#ifndef __SASC
  DOSBase = (struct DosLibrary *)OpenLibrary((STRPTR) "dos.library", 0L);
#endif

  /* First parse args */
  args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
  if (args == NULL)
  {
    PutStr(TEMPLATE);
    PutStr("  Invalid Args!\n");
    return RETURN_ERROR;
  }

  /* set options */
  if (params.device != NULL)
  {
    device = params.device;
  }
  if (params.unit != NULL)
  {
    unit = *params.unit;
  }

  result = plipbench(device, unit);

  /* free args */
  FreeArgs(args);

#ifndef __SASC
  CloseLibrary((struct Library *)DOSBase);
#endif

  return result;
}
