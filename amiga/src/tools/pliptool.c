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
  "-V=VERBOSE/S,"
  "LOAD_PREFS/S,"
  "SAVE_PREFS/S,"
  "RESET_PREFS/S,"
  "SET_MAC/K,"
  "SET_MODE/K,"
  "SET_FLAGS/K"
  ;
typedef struct {
  char  *device;
  ULONG *unit;
  ULONG verbose;
  ULONG load_prefs;
  ULONG save_prefs;
  ULONG reset_prefs;
  char  *set_mac;
  char  *set_mode;
  char  *set_flags;
} params_t;
static params_t params;

/* ----- plipbox infos ----- */
BOOL get_version_info(sanadev_handle_t *sh)
{
  BOOL ok;
  UWORD dev_version = 0;
  UWORD fw_version = 0;

  /* retrieve and show device and firmware version */
  ok = sanadev_cmd_plipbox_get_version(sh, &dev_version, &fw_version);
  if(!ok) {
    PutStr("Error retrieving version info from device! No plipbox device?\n");
    return FALSE;
  }
  Printf("Versions\n");
  Printf("   device: %04lx\n", (ULONG)dev_version);
  Printf(" firmware: %04lx\n", (ULONG)fw_version);
  return TRUE;
}

BOOL process_cmds(sanadev_handle_t *sh)
{
  sanadev_plipbox_param_t plipbox_param;
  BOOL ok;
  UWORD status;

  // do we need to load or reset the params from flash on device?
  if(params.load_prefs) {
    PutStr("Loading device parameters from flash...");
    ok = sanadev_cmd_plipbox_load_prefs(sh, &status);
    Printf("result=%lx\n", (ULONG)status);
    if(!ok) {
      return FALSE;
    }
  }
  else if(params.reset_prefs) {
    PutStr("Reset device parameters to factory defaults...");
    ok = sanadev_cmd_plipbox_reset_prefs(sh, &status);
    Printf("result=%lx\n", (ULONG)status);
    if(!ok) {
      return FALSE;
    }
  }

  // now read params
  PutStr("Reading current paramters...\n");
  ok = sanadev_plipbox_read_param(sh, &plipbox_param);
  if(!ok) {
    return FALSE;
  }

  // apply changes?
  UWORD update_flags = 0;

  // set mac
  if(params.set_mac != NULL) {
    Printf("Setting new MAC address: %s\n", params.set_mac);
    ok = sanadev_parse_mac(params.set_mac, plipbox_param.cur_mac);
    if(!ok) {
      Printf("Error parsing MAC address!\n");
      return FALSE;
    }
    update_flags = SANADEV_UPDATE_MAC;
  }

  // set mode
  if(params.set_mode != NULL) {
    Printf("Setting new mode: %s\n", params.set_mode);
    ok = sanadev_parse_mode(params.set_mode, &plipbox_param.mode);
    if(!ok) {
      Printf("Error parsing mode!\n");
      return FALSE;
    }
    update_flags |= SANADEV_UPDATE_MODE;
  }

  // set flags
  if(params.set_flags != NULL) {
    Printf("Setting new flags: %s\n", params.set_flags);
    ok = sanadev_parse_flags(params.set_flags, &plipbox_param.flags);
    if(!ok) {
      Printf("Error parsing flags!\n");
      return FALSE;
    }
    update_flags |= SANADEV_UPDATE_FLAGS;
  }

  // need to write param changes to device?
  if(update_flags != 0) {
    PutStr("Writing changes to device...\n");
    ok = sanadev_plipbox_write_param(sh, &plipbox_param, update_flags);
    if(!ok) {
      return FALSE;
    }
  }

  // do we need to persist changes?
  if(params.save_prefs) {
    PutStr("Saving device parameters to flash...");
    ok = sanadev_cmd_plipbox_save_prefs(sh, &status);
    Printf("result=%lx\n", (ULONG)status);
    if(!ok) {
      return FALSE;
    }
  }

  // finally print new parameters
  sanadev_plipbox_print_param(&plipbox_param);

  return TRUE;
}

/* ----- tool ----- */
int pliptool(const char *device, LONG unit)
{
  sanadev_handle_t *sh;
  UWORD error;
  BOOL ok;

  LOG(("Opening device '%s' unit #%ld\n", device, unit));
  sh = sanadev_open(device, unit, 0, &error);
  if(sh == NULL) {
    Printf("Error opening device '%s' unit #%ld: code=%ld\n", device, unit, error);
    return RETURN_ERROR;
  }

  // first try to get version info
  ok = get_version_info(sh);
  if(ok) {
    ok = process_cmds(sh);
    if(!ok) {
      PutStr("Operation failed!\n");
      sanadev_cmd_print_error(sh);
    }
  }

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
