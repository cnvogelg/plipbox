#include <exec/exec.h>
#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include "sanadev.h"
#include "param_tag.h"
#include "param_shared.h"
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
    "LOOPBACK_DEV/S,"
    "LOOPBACK_MAC/S,"
    "DIRECT_SPI/S,"
    "LOOPS/N/K";
typedef struct
{
  char *device;
  ULONG *unit;
  ULONG verbose;
  ULONG loopback_buf;
  ULONG loopback_dev;
  ULONG loopback_mac;
  ULONG direct_spi;
  ULONG *loops;
} params_t;
static params_t params;

static UWORD old_mode;
static UWORD old_flag;

static BOOL set_mode_and_flags(sanadev_handle_t *sh)
{
  BOOL ok;

  // save old mode
  ok = param_tag_mode_get(sh, &old_mode);
  if (!ok)
  {
    return FALSE;
  }
  LOG(("Old Mode: %ld\n", (LONG)old_mode));

  // set new mode
  UWORD mode = 0;
  if (params.loopback_buf)
  {
    LOG(("Mode: loopback with plipbox buffer\n"));
    mode = PARAM_MODE_LOOPBACK_BUF;
  }
  else if (params.loopback_dev)
  {
    LOG(("Mode: loopback with PIO buffer\n"));
    mode = PARAM_MODE_LOOPBACK_DEV;
  }
  else if (params.loopback_mac)
  {
    LOG(("Mode: loopback with PIO MAC\n"));
    mode = PARAM_MODE_LOOPBACK_MAC;
  }
  LOG(("Setting mode: %ld\n", (LONG)mode));
  ok = param_tag_mode_set(sh, mode);
  if (!ok)
  {
    return FALSE;
  }

  // save old flag
  ok = param_tag_flag_get(sh, &old_flag);
  if (!ok)
  {
    return FALSE;
  }
  LOG(("Old Flag: %ld\n", (LONG)old_flag));

  // set new flag
  UWORD flag = 0;
  if (params.direct_spi)
  {
    LOG(("Flag: direct SPI transfer to PIO\n"));
    flag = PARAM_FLAG_PIO_DIRECT_SPI;
  }
  LOG(("Setting flag: %ld\n", (LONG)flag));
  ok = param_tag_flag_set(sh, flag);
  return ok;
}

static BOOL restore_mode_and_flags(sanadev_handle_t *sh)
{
  BOOL ok;

  LOG(("Restore Mode: %ld\n", (LONG)old_mode));
  ok = param_tag_mode_set(sh, old_mode);
  if (!ok)
  {
    return FALSE;
  }

  LOG(("Restore Flag: %ld\n", (LONG)old_flag));
  ok = param_tag_flag_set(sh, old_flag);
  return ok;
}

static void setup_bench_opt(bench_opt_t *opt)
{
  if (params.loops != NULL)
  {
    opt->loops = *params.loops;
  }
  else
  {
    opt->loops = 0;
  }
  LOG(("Bench Loops: %ld\n", opt->loops));
}

/* ----- tool ----- */
static int plipbench(const char *device, LONG unit)
{
  sanadev_handle_t *sh;
  atimer_handle_t *th;
  UWORD error;
  BOOL ok;
  bench_opt_t bench_opt;

  // parse options
  setup_bench_opt(&bench_opt);

  // setup timer
  LOG(("Opening timer!\n"));
  th = atimer_init((struct Library *)SysBase);
  if (th == NULL)
  {
    PutStr("Error: no timer!\n");
    return RETURN_ERROR;
  }
  ok = atimer_sig_init(th);
  if (!ok)
  {
    PutStr("Error: no timer signal!\n");
    atimer_exit(th);
    return RETURN_ERROR;
  }

  // open plipbox.device
  LOG(("Opening device '%s' unit #%ld\n", device, unit));
  sh = sanadev_open(device, unit, 0, &error);
  if (sh == NULL)
  {
    Printf("Error opening device '%s' unit #%ld: code=%ld\n", device, unit, (LONG)error);
    return RETURN_ERROR;
  }

  // set mode and flag
  ok = set_mode_and_flags(sh);
  if (ok)
  {

    // setup events
    ok = sanadev_event_init(sh, &error);
    if (!ok)
    {
      Printf("Error setting up SANA-II events! code=%ld\n", (LONG)error);
    }
    else
    {

      // go online
      PutStr("going online...\n");
      Flush(Output());
      ok = sanadev_cmd_online(sh);
      if (!ok)
      {
        PutStr("Error going online!\n");
        sanadev_cmd_print_error(sh);
      }
      else
      {

#if 1
        PutStr("waiting for online...!\n");
        Flush(Output());
        sanadev_event_start(sh, S2EVENT_ONLINE);
        atimer_sig_start(th, 5, 0);
        ULONG sana_mask = sanadev_event_get_mask(sh);
        ULONG timer_mask = atimer_sig_get_mask(th);
        ULONG mask = sana_mask | timer_mask | SIGBREAKF_CTRL_C;
        Printf("sana_mask=%lx timer_mask=%lx\n", sana_mask, timer_mask);
        ULONG got_mask = Wait(mask);
        Printf("got_mask=%lx\n", got_mask);

        // got sana event
        if ((got_mask & sana_mask) == sana_mask)
        {
          // wait for online event
          ULONG event;
          BOOL ok = sanadev_event_get_event(sh, &event);

          LOG(("Got ok=%ld event: %lx\n", (ULONG)ok, event));
          if ((event & S2EVENT_ONLINE) == S2EVENT_ONLINE)
          {
            PutStr("we are online!\n");

            // we made it: enter main loop
            bench_loop(sh, &bench_opt);
          }
        }

        atimer_sig_stop(th);
        sanadev_event_stop(sh);
#endif

        // finally go offline
        PutStr("going offline...\n");
        ok = sanadev_cmd_offline(sh);
        if (!ok)
        {
          PutStr("Error going offline!\n");
          sanadev_cmd_print_error(sh);
        }
      }

      LOG(("Sana Event exit\n"));
      sanadev_event_exit(sh);
    }

    // return to old mode
    restore_mode_and_flags(sh);
  }
  else
  {
    PutStr("Error setting mode and flag and device...\n");
  }

  LOG(("Closing device...\n"));
  sanadev_close(sh);
  LOG(("Done\n"));

  LOG(("Freeing timer\n"));
  atimer_sig_exit(th);
  atimer_exit(th);

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
