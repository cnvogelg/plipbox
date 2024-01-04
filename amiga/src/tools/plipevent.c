#include <exec/exec.h>
#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>

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

typedef struct
{
  ULONG event;
  char *desc;
} event_desc_t;
static const event_desc_t event_desc[] = {
  { S2EVENT_ONLINE, "Online" },
  { S2EVENT_OFFLINE, "Offline" },
  { 0, NULL }
};

static void print_ts(atime_stamp_t *ts)
{
  Printf("%08ld.%06ld ", ts->s, ts->ms);
}

static void print_events(ULONG events)
{
  const event_desc_t *ptr = &event_desc[0];
  while(ptr->event != 0) {
    if((events & ptr->event) == ptr->event) {
      PutStr(ptr->desc);
      PutStr(" ");
    }
    ptr++;
  }
  PutStr("\n");
}

/* ----- tool ----- */
static int plipevent(const char *device, LONG unit)
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
    Printf("Error opening device '%s' unit #%ld: code=%ld\n", device, unit, (LONG)error);
    return RETURN_ERROR;
  }

  // init events
  ok = sanadev_event_init(sh, &error);
  if(!ok) {
    sanadev_close(sh);

    Printf("Error init events. code=%ld\n", (LONG)error);
    return RETURN_ERROR;
  }

  PutStr("waiting for events... press Ctrl+C to quit.\n");
  Flush(Output());

  ULONG other_events = S2EVENT_ERROR | S2EVENT_TX | S2EVENT_RX |
    S2EVENT_BUFF | S2EVENT_HARDWARE | S2EVENT_SOFTWARE;
  BOOL is_online = FALSE;

  // start timing
  atime_stamp_t  start_time;
  atimer_sys_time_get(th, &start_time);

  while(1) {

    // setup events to wait for
    ULONG events;
    if(is_online) {
      events = other_events | S2EVENT_OFFLINE;
    } else {
      events = other_events | S2EVENT_ONLINE;
    }

    LOG(("Start events: %lx\n", events));
    sanadev_event_start(sh, events);
    ULONG sana_mask = sanadev_event_get_mask(sh);
    ULONG mask = sana_mask | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D;
    ULONG got_mask = Wait(mask);
    LOG(("Wait got_mask=%lx\n", got_mask));

    // timing
    atime_stamp_t  now;
    atime_stamp_t  delta;

    // got sana event
    if ((got_mask & sana_mask) == sana_mask)
    {
      // pick up event
      ULONG got_events;
      BOOL ok = sanadev_event_result(sh, &got_events);

      LOG(("Got ok=%ld event: %lx\n", (ULONG)ok, got_events));
      if(ok) {
        atimer_sys_time_get(th, &now);
        atimer_sys_time_delta(&now, &start_time, &delta);
        print_ts(&delta);
        print_events(got_events);
      }

      // update online state
      if((got_events & S2EVENT_ONLINE) == S2EVENT_ONLINE) {
        is_online = TRUE;
      }
      if((got_events & S2EVENT_OFFLINE) == S2EVENT_OFFLINE) {
        is_online = FALSE;
      }
    }
    else {
      LOG(("Stop events\n"));
      sanadev_event_stop(sh);
    }
    if ((got_mask & (SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_C)) != 0) {
      PutStr("bye...\n");
      break;
    }
  }

  LOG(("Exit events\n"));
  sanadev_event_exit(sh);

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

  result = plipevent(device, unit);

  /* free args */
  FreeArgs(args);

#ifndef __SASC
  CloseLibrary((struct Library *)DOSBase);
#endif

  return result;
}
