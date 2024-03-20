#include <exec/exec.h>
#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include "sanadev.h"
#include "param.h"
#include "plipbox_req.h"
#include "plipbox_cmd.h"

#define LOG(x)          \
  do                    \
  {                     \
    if (params.verbose) \
    {                   \
      Printf x;         \
    }                   \
  } while (0)

/* lib bases */
extern struct ExecBase *SysBase;
#ifdef __SASC
extern struct DosLibrary *DOSBase;
#else
struct DosLibrary *DOSBase;
#endif

#define PARAM_STR_BUF_SIZE 256
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
    "DUMP/S";
typedef struct
{
  char *device;
  ULONG *unit;
  ULONG verbose;
  ULONG prefs_load;
  ULONG prefs_save;
  ULONG prefs_reset;
  char *param_tag;
  ULONG *param_id;
  char *param_set;
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

  /* is a newstyle device? */
  ok = sanadev_is_newstyle(sh);
  Printf("Newstyle Device: %s\n", ok ? "yes": "no");

  /* retrieve and show device and firmware version */
  ok = plipbox_cmd_get_version(sh, &dev_version, &fw_version);
  if (!ok)
  {
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

static BOOL dump_param(sanadev_handle_t *sh, param_def_t *def)
{
  // alloc data buffer
  UBYTE *data = AllocVec(def->size, MEMF_ANY | MEMF_CLEAR);
  if (data == NULL)
  {
    Printf("No memory!\n");
    return FALSE;
  }

  // get param
  int res = plipbox_req_param_get_val(sh, def->index, def->size, data);
  if (res == REQ_OK)
  {
    // print
    res = param_print_val(print_buffer, def, data);
    if (res == PARAM_PARSE_OK)
    {
      PutStr(print_buffer);
    }
    else
    {
      Printf("Error printing value: %s\n", param_parse_perror(res));
    }
  }
  else
  {
    Printf("Error %ld getting parameter #%ld!\n", (ULONG)res, (ULONG)def->index);
  }

  FreeVec(data);
  return (res == REQ_OK);
}

static BOOL set_param(sanadev_handle_t *sh, param_def_t *def, const char *txt)
{
  // alloc data buffer
  UBYTE *data = AllocVec(def->size, MEMF_ANY | MEMF_CLEAR);
  if (data == NULL)
  {
    Printf("No memory!\n");
    return FALSE;
  }

  // parse param
  int res = param_parse_val(txt, def, data);
  if (res == PARAM_PARSE_OK)
  {
    // set param
    res = plipbox_req_param_set_val(sh, def->index, def->size, data);
    if (res != REQ_OK)
    {
      Printf("Error %ld setting parameter #%ld!\n", (ULONG)res, (ULONG)def->index);
    }
  }
  else
  {
    Printf("Error parsing parameter #%ld: '%s' -> %s\n", (ULONG)def->index, txt,
           param_parse_perror(res));
    res = REQ_ERROR_UNKNOWN;
  }

  FreeVec(data);
  return (res == REQ_OK);
}

static BOOL dump_params(sanadev_handle_t *sh)
{
  UWORD i;
  UBYTE num_param = 0;
  int res = plipbox_req_param_get_num(sh, &num_param);
  if (res != REQ_OK)
  {
    Printf("Error %ld getting number of parameters from device!\n", (ULONG)res);
    return FALSE;
  }

  Printf("Number of Parameters: %ld\n", (ULONG)num_param);
  for (i = 0; i < num_param; i++)
  {
    param_def_t def;
    res = plipbox_req_param_get_def(sh, i, &def);
    if (res != REQ_OK)
    {
      Printf("Error getting param definition #%ld\n", (ULONG)i);
      return FALSE;
    }

    dump_param(sh, &def);
  }

  return TRUE;
}

static BOOL process_cmds(sanadev_handle_t *sh)
{
  int res;
  UWORD status;

  // do we need to load or reset the params from flash on device?
  if (params.prefs_load)
  {
    PutStr("Loading device parameters from flash...");
    res = plipbox_req_prefs_load(sh);
    Printf("result=%ld\n", (ULONG)res);
    if (res != REQ_OK)
    {
      return FALSE;
    }
  }
  // or do we reset them to factory defaults?
  else if (params.prefs_reset)
  {
    PutStr("Reset device parameters to factory defaults...");
    res = plipbox_req_prefs_reset(sh);
    Printf("result=%ld\n", (ULONG)res);
    if (res != REQ_OK)
    {
      return FALSE;
    }
  }

  // process a parameter?
  UBYTE index = PARAM_ID_INVALID;
  // given by tag
  if (params.param_tag != NULL)
  {
    ULONG tag;
    if (param_parse_tag(params.param_tag, &tag) == PARAM_PARSE_OK)
    {
      Printf("Searching tag '%s' (%04lx)\n", params.param_tag, tag);
      res = plipbox_req_param_find_tag(sh, tag, &index);
      if (res != REQ_OK)
      {
        Printf("Search failed with Error %ld\n", res);
        return FALSE;
      }
      if (index == PARAM_ID_INVALID)
      {
        PutStr("Tag not found!\n");
        return TRUE;
      }
    }
    else
    {
      Printf("Invalid tag given: '%s'\n", params.param_tag);
      return TRUE;
    }
  }
  // given by index
  else if (params.param_id != NULL)
  {
    index = (UWORD)*params.param_id;
    UBYTE num_param = 0;
    res = plipbox_req_param_get_num(sh, &num_param);
    if (res != REQ_OK)
    {
      Printf("Error %ld getting number of parameters from device!\n", (ULONG)res);
      return FALSE;
    }
    if (index >= num_param)
    {
      Printf("Index out of range: %ld >= %ld\n", (ULONG)index, (ULONG)num_param);
      return TRUE;
    }
  }

  // set or dump value?
  if (index != PARAM_ID_INVALID)
  {
    // get param def
    param_def_t param_def;
    res = plipbox_req_param_get_def(sh, index, &param_def);
    if (res != REQ_OK)
    {
      Printf("Error %ld getting param definition #%ld\n", (ULONG)res, (ULONG)index);
      return FALSE;
    }

    // set
    if (params.param_set != NULL)
    {
      BOOL ok = set_param(sh, &param_def, params.param_set);
      if (!ok)
      {
        return FALSE;
      }
    }
    // dump
    BOOL ok = dump_param(sh, &param_def);
    if (!ok)
    {
      return FALSE;
    }
  }

  // dump all params
  if (params.param_dump)
  {
    BOOL ok = dump_params(sh);
    if (!ok)
    {
      return FALSE;
    }
  }

  // do we need to persist changes?
  if (params.prefs_save)
  {
    PutStr("Saving device parameters to flash...");
    res = plipbox_req_prefs_save(sh);
    Printf("result=%ld\n", (ULONG)res);
    if (res != REQ_OK)
    {
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
  if (sh == NULL)
  {
    Printf("Error opening device '%s' unit #%ld: code=%ld\n", device, unit, error);
    return RETURN_ERROR;
  }

  // first try to get version info
  ok = get_device_info(sh);
  if (ok)
  {
    ok = process_cmds(sh);
    if (!ok)
    {
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

  result = pliptool(device, unit);

  /* free args */
  FreeArgs(args);

#ifndef __SASC
  CloseLibrary((struct Library *)DOSBase);
#endif

  return result;
}
