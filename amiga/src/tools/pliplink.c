#include <exec/exec.h>
#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include "devices/sana2link.h"

#include "sanadev.h"
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
    "-V=VERBOSE/S";
typedef struct
{
  char *device;
  ULONG *unit;
  ULONG verbose;
} params_t;
static params_t params;

static void print_ts(atime_stamp_t *ts)
{
  Printf("%08ld.%06ld ", ts->s, ts->ms);
}

static void print_link_status(BYTE link_status)
{
  switch(link_status) {
  case S2LINKSTATUS_UP:
    Printf("Link up\n");
    break;
  case S2LINKSTATUS_DOWN:
    Printf("Link down\n");
    break;
  case S2LINKSTATUS_UNKNOWN:
    Printf("Link status unknown\n");
    break;
  default:
    Printf("INVALID VALUE: %lx\n", (ULONG)link_status);
    break;
  }
}

/* ----- tool ----- */
static int pliplink(const char *device, LONG unit)
{
  sanadev_handle_t *sh;
  atimer_handle_t *th;
  UWORD error;
  BOOL ok;

  // open timer
  LOG(("Open timer\n"));
  th = atimer_init((struct Library *)SysBase);
  if(th == NULL) {
    Printf("Error opening timer!\n");
    return RETURN_ERROR;
  }

  // open plipbox.device
  LOG(("Opening device '%s' unit #%ld\n", device, unit));
  sh = sanadev_open(device, unit, 0, &error);
  if (sh == NULL)
  {
    atimer_exit(th);

    Printf("Error opening device '%s' unit #%ld: code=%ld\n", device, unit, (LONG)error);
    return RETURN_ERROR;
  }

  // check if link status is available in device?
  LOG(("Checking for link status support in device!\n"));
  BOOL has_link_status = sanadev_link_is_supported(sh);
  if(!has_link_status) {
    atimer_exit(th);
    sanadev_close(sh);

    PutStr("Link Status Command not supported by device. Aborting.\n");
    return RETURN_ERROR;
  }

  // init link status
  ok = sanadev_link_init(sh, &error);
  if(!ok) {
    atimer_exit(th);
    sanadev_close(sh);

    Printf("Error init link status handling. code=%ld\n", (LONG)error);
    return RETURN_ERROR;
  }

  PutStr("waiting for link status... press Ctrl+C to quit.\n");
  Flush(Output());

  // start timing
  atime_stamp_t  start_time;
  atimer_sys_time_get(th, &start_time);

  while(1) {

    LOG(("Start link status\n"));
    sanadev_link_start(sh);
    ULONG sana_mask = sanadev_link_get_mask(sh);
    ULONG mask = sana_mask | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D;
    ULONG got_mask = Wait(mask);
    LOG(("Wait got_mask=%lx\n", got_mask));

    // timing
    atime_stamp_t  now;
    atime_stamp_t  delta;

    // got sana link status update
    if ((got_mask & sana_mask) == sana_mask)
    {
      // pick up link status
      BYTE new_link_status = S2LINKSTATUS_UNKNOWN;
      BOOL ok = sanadev_link_result(sh, &new_link_status);

      LOG(("Got ok=%ld link status: %ld\n", (ULONG)ok, (LONG)new_link_status));
      if(ok) {
        atimer_sys_time_get(th, &now);
        atimer_sys_time_delta(&now, &start_time, &delta);
        print_ts(&delta);
        print_link_status(new_link_status);
      }
    }
    if ((got_mask & (SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_C)) != 0) {
      PutStr("bye...\n");
      break;
    }
  }

  LOG(("Stop link status\n"));
  sanadev_link_stop(sh);

  LOG(("Exit link status\n"));
  sanadev_link_exit(sh);

  LOG(("Closing device...\n"));
  sanadev_close(sh);

  LOG(("Closing timer...\n"));
  atimer_exit(th);

  LOG(("Done\n"));
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

  result = pliplink(device, unit);

  /* free args */
  FreeArgs(args);

#ifndef __SASC
  CloseLibrary((struct Library *)DOSBase);
#endif

  return result;
}
