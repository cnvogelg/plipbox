#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "param_tag.h"
#include "param_shared.h"
#include "plipbox_cmd.h"

// mode

BOOL param_tag_mode_set(sanadev_handle_t *sh, UWORD mode)
{
  UWORD id = 0;
  BOOL ok = plipbox_cmd_param_find_tag(sh, PARAM_TAG_MODE, &id);
  if (!ok)
  {
    return FALSE;
  }
  return plipbox_cmd_param_set_word(sh, id, mode);
}

BOOL param_tag_mode_get(sanadev_handle_t *sh, UWORD *mode)
{
  UWORD id = 0;
  BOOL ok = plipbox_cmd_param_find_tag(sh, PARAM_TAG_MODE, &id);
  if (!ok)
  {
    return FALSE;
  }
  return plipbox_cmd_param_get_word(sh, id, mode);
}

// ncap

BOOL param_tag_ncap_set(sanadev_handle_t *sh, UWORD flag)
{
  UWORD id = 0;
  BOOL ok = plipbox_cmd_param_find_tag(sh, PARAM_TAG_NCAP, &id);
  if (!ok)
  {
    return FALSE;
  }
  return plipbox_cmd_param_set_word(sh, id, flag);
}

BOOL param_tag_ncap_get(sanadev_handle_t *sh, UWORD *flag)
{
  UWORD id = 0;
  BOOL ok = plipbox_cmd_param_find_tag(sh, PARAM_TAG_NCAP, &id);
  if (!ok)
  {
    return FALSE;
  }
  return plipbox_cmd_param_get_word(sh, id, flag);
}
