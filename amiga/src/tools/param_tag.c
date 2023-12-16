#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "param_tag.h"
#include "param_shared.h"

// mode

BOOL param_tag_mode_set(sanadev_handle_t *sh, UWORD mode)
{
  UWORD id = 0;
  BOOL ok = sanadev_cmd_plipbox_param_find_tag(sh, PARAM_TAG_MODE, &id);
  if(!ok) {
    return FALSE;
  }
  return sanadev_cmd_plipbox_param_set_val(sh, id, sizeof(mode), (UBYTE *)&mode);
}

BOOL param_tag_mode_get(sanadev_handle_t *sh, UWORD *mode)
{
  UWORD id = 0;
  BOOL ok = sanadev_cmd_plipbox_param_find_tag(sh, PARAM_TAG_MODE, &id);
  if(!ok) {
    return FALSE;
  }
  return sanadev_cmd_plipbox_param_get_val(sh, id, sizeof(mode), (UBYTE *)&mode);
}

// flag

BOOL param_tag_flag_set(sanadev_handle_t *sh, UWORD flag)
{
  UWORD id = 0;
  BOOL ok = sanadev_cmd_plipbox_param_find_tag(sh, PARAM_TAG_FLAG, &id);
  if(!ok) {
    return FALSE;
  }
  return sanadev_cmd_plipbox_param_set_val(sh, id, sizeof(flag), (UBYTE *)&flag);
}

BOOL param_tag_flag_get(sanadev_handle_t *sh, UWORD *flag)
{
  UWORD id = 0;
  BOOL ok = sanadev_cmd_plipbox_param_find_tag(sh, PARAM_TAG_FLAG, &id);
  if(!ok) {
    return FALSE;
  }
  return sanadev_cmd_plipbox_param_get_val(sh, id, sizeof(flag), (UBYTE *)&flag);
}

