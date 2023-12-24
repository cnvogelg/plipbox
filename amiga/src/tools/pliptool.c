#include <exec/exec.h>
#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include "sanadev.h"
#include "param.h"
#include "plipbox_cmd.h"

#define LOG(x) do { if(params.verbose) { Printf x ; } } while(0)

/* lib bases */
extern struct ExecBase *SysBase;
#ifdef __SASC
extern struct DosLibrary *DOSBase;
#else
struct DosLibrary *DOSBase;
#endif

#define PARAM_STR_BUF_SIZE  256
char print_buffer[PARAM_STR_BUF_SIZE];

/* params */
static const char *TEMPLATE =
  "-D=DEVICE/K,"
  "-U=UNIT/N/K,"
  "-V=VERBOSE/S,"
  "PREFS_LOAD/S,"
  "PREFS_SAVE/S,"
  "PREFS_RESET/S,"
  "TAG/K,"
  "ID/N/K,"
  "SET/K,"
  "DUMP/S"
  ;
typedef struct {
  char  *device;
  ULONG *unit;
  ULONG verbose;
  ULONG prefs_load;
  ULONG prefs_save;
  ULONG prefs_reset;
  char  *param_tag;
  ULONG *param_id;
  char  *param_set;
  ULONG param_dump;
} params_t;
static params_t params;

/* ----- plipbox infos ----- */
static BOOL get_device_info(sanadev_handle_t *sh)
{
  BOOL ok;
  UWORD dev_version = 0;
  UWORD fw_version = 0;
  sanadev_mac_t cur_mac;
  sanadev_mac_t def_mac;

  /* retrieve and show device and firmware version */
  ok = plipbox_cmd_get_version(sh, &dev_version, &fw_version);
  if(!ok) {
    PutStr("Error retrieving version info from device! No plipbox device?\n");
    return FALSE;
  }
  Printf("Versions\n");
  Printf("   device:   %04lx\n", (ULONG)dev_version);
  Printf(" firmware:   %04lx\n", (ULONG)fw_version);

  /* mac addresses */
  ok = sanadev_cmd_get_station_address(sh, cur_mac, def_mac);
  Printf("current MAC: ");
  sanadev_print_mac(cur_mac);
  Printf("\ndefault MAC: ");
  sanadev_print_mac(def_mac);
  Printf("\n");

  return TRUE;
}

static BOOL dump_param(sanadev_handle_t *sh, s2pb_param_def_t *def)
{
  // alloc data buffer
  UBYTE *data = AllocVec(def->size, MEMF_ANY | MEMF_CLEAR);
  if(data == NULL) {
    Printf("No memory!\n");
    return FALSE;
  }

  // get param
  BOOL ok = plipbox_cmd_param_get_val(sh, def->index, def->size, data);
  if(ok) {
    // print
    int res = param_print_val(print_buffer, def, data);
    if(res == PARAM_OK) {
      PutStr(print_buffer);
    } else {
      Printf("Error printing value: %s\n", param_perror(res));
    }
  } else {
    Printf("Error getting parameter #%ld!\n", (ULONG)def->index);
  }

  FreeVec(data);
  return ok;
}

static BOOL set_param(sanadev_handle_t *sh, s2pb_param_def_t *def, const char *txt)
{
  // alloc data buffer
  UBYTE *data = AllocVec(def->size, MEMF_ANY | MEMF_CLEAR);
  if(data == NULL) {
    Printf("No memory!\n");
    return FALSE;
  }

  // parse param
  BOOL ok = TRUE;
  int res = param_parse_val(txt, def, data);
  if(res == PARAM_OK) {
    // set param
    ok = plipbox_cmd_param_set_val(sh, def->index, def->size, data);
    if(!ok) {
      Printf("Error setting parameter #%ld!\n", (ULONG)def->index);
    }
  } else {
    Printf("Error parsing parameter #%ld: '%s' -> %s\n", (ULONG)def->index, txt,
          param_perror(res));
    ok = FALSE;
  }

  FreeVec(data);
  return ok;
}

static BOOL dump_params(sanadev_handle_t *sh)
{
  UWORD i;
  UWORD num_param = 0;
  BOOL ok = plipbox_cmd_param_get_num(sh, &num_param);
  if(!ok) {
    Printf("Error getting number of parameters from device!\n");
    return FALSE;
  }

  for(i=0;i<num_param;i++) {
    s2pb_param_def_t def;
    ok = plipbox_cmd_param_get_def(sh, i, &def);
    if(!ok) {
      Printf("Error getting param definition #%ld\n", (ULONG)i);
      return FALSE;
    }

    dump_param(sh, &def);
  }

  return TRUE;
}

static BOOL process_cmds(sanadev_handle_t *sh)
{
  BOOL ok;
  UWORD status;

  // do we need to load or reset the params from flash on device?
  if(params.prefs_load) {
    PutStr("Loading device parameters from flash...");
    ok = plipbox_cmd_prefs_load(sh, &status);
    Printf("result=%lx\n", (ULONG)status);
    if(!ok) {
      return FALSE;
    }
  }
  // or do we reset them to factory defaults?
  else if(params.prefs_reset) {
    PutStr("Reset device parameters to factory defaults...\n");
    ok = plipbox_cmd_prefs_reset(sh);
    if(!ok) {
      return FALSE;
    }
  }

  // process a parameter?
  UWORD index = S2PB_NO_INDEX;
  // given by tag
  if(params.param_tag != NULL) {
    ULONG tag;
    if(param_parse_tag(params.param_tag, &tag) == PARAM_OK) {
      Printf("Searching tag '%s' (%04lx)\n", params.param_tag, tag);
      ok = plipbox_cmd_param_find_tag(sh, tag, &index);
      if(!ok) {
        return FALSE;
      }
      if(index == S2PB_NO_INDEX) {
        PutStr("Tag not found!\n");
        return TRUE;
      }
    } else {
      Printf("Invalid tag given: '%s'\n", params.param_tag);
      return TRUE;
    }
  }
  // given by index
  else if(params.param_id != NULL) {
    index = (UWORD)*params.param_id;
    UWORD num_param = 0;
    ok = plipbox_cmd_param_get_num(sh, &num_param);
    if(!ok) {
      Printf("Error getting number of parameters from device!\n");
      return FALSE;
    }
    if(index >= num_param) {
      Printf("Index out of range: %ld >= %ld\n", (ULONG)index, (ULONG)num_param);
      return TRUE;
    }
  }

  // set or dump value?
  if(index != S2PB_NO_INDEX) {
    // get param def
    s2pb_param_def_t param_def;
    ok = plipbox_cmd_param_get_def(sh, index, &param_def);
    if(!ok) {
      Printf("Error getting param definition #%ld\n", (ULONG)index);
      return FALSE;
    }

    // set
    if(params.param_set != NULL) {
      ok = set_param(sh, &param_def, params.param_set);
      if(!ok) {
        return FALSE;
      }
    }
    // dump
    ok = dump_param(sh, &param_def);
    if(!ok) {
      return FALSE;
    }
  }

  // dump all params
  if(params.param_dump) {
    ok = dump_params(sh);
    if(!ok) {
      return FALSE;
    }
  }

  // do we need to persist changes?
  if(params.prefs_save) {
    PutStr("Saving device parameters to flash...");
    ok = plipbox_cmd_prefs_save(sh, &status);
    Printf("result=%lx\n", (ULONG)status);
    if(!ok) {
      return FALSE;
    }
  }

  return TRUE;
}

/* ----- tool ----- */
static int pliptool(const char *device, LONG unit)
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
  ok = get_device_info(sh);
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
