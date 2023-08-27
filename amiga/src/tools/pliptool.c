#include <exec/exec.h>
#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include "sanadev.h"

#define LOG(x) do { if(params.verbose) { Printf x ; } } while(0)

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
  "-V=VERBOSE/S"
  ;
typedef struct {
  char  *device;
  ULONG *unit;
  ULONG verbose;
} params_t;
static params_t params;

/* ----- plipbox infos ----- */
BOOL show_info(sanadev_handle_t *sh)
{
  BOOL ok;
  UWORD dev_version;
  UWORD fw_version;

  /* retrieve and show device and firmware version */
  ok = sanadev_cmd_get_version(sh, &dev_version, &fw_version);
  if(!ok) {
    PutStr("Error retrieving version info from device!\n");
    return FALSE;
  }
  Printf("Versions\n")
  Printf("   device: %04lx\n", (ULONG)dev_version);
  Printf(" firmware: %04lx\n", (ULONG)fw_version);

  /* retrieve mac adresses */
  sanadev_mac_t cur_mac;
  sanadev_mac_t def_mac;
  ok = sanadev_cmd_get_station_address(sh, cur_mac, def_mac);
  if(!ok) {
    PutStr("Error retrieving MAC addresses from device!\n");
    return FALSE;
  }
  Printf("\nMAC Addresses\n  current: ");
  sanadev_print_mac(cur_mac);
  Printf("\n  default: ");
  sanadev_print_mac(def_mac);
  PutStr("\n");

  return TRUE;
}

/* ----- tool ----- */
int pliptool(const char *device, LONG unit)
{
  sanadev_handle_t *sh;
  UWORD error;

  LOG(("Opening device '%s' unit #%ld\n", device, unit));
  sh = sanadev_open(device, unit, 0, &error);
  if(sh == NULL) {
    Printf("Error opening device '%s' unit #%ld: code=%ld\n", device, unit, error);
    return RETURN_ERROR;
  }

  show_info(sh);


  LOG(("Closing device...\n"));
  sanadev_close(sh);
  LOG(("Done\n"));

  return RETURN_OK;
}

/* ---------- main ---------- */
int main(void)
{
  struct RDArgs *args;
  char *device = "plipbox.device";
  LONG  unit = 0;
  int   result;

#ifndef __SASC
  DOSBase = (struct DosLibrary *)OpenLibrary((STRPTR)"dos.library", 0L);
#endif

  /* First parse args */
  args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
  if(args == NULL) {
    PutStr(TEMPLATE);
    PutStr("  Invalid Args!\n");
    return RETURN_ERROR;
  }

  /* set options */
  if(params.device != NULL) {
    device = params.device;
  }
  if(params.unit != NULL) {
    unit = *params.unit;
  }

  result = pliptool(device, unit);

  /* free args */
  FreeArgs(args);

#ifndef __SASC
  CloseLibrary((struct Library *)DOSBase);
#endif

  return result;
}
