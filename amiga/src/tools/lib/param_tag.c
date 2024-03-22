#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "param_tag.h"
#include "param_shared.h"
#include "plipbox_req.h"

// mode

int param_tag_mode_set(sanadev_handle_t *sh, UWORD mode)
{
  UBYTE id = 0;
  int res = plipbox_req_param_find_tag(sh, PARAM_TAG_MODE, &id);
  if (res != REQ_OK)
  {
    return res;
  }
  return plipbox_req_param_set_word(sh, id, mode);
}

int param_tag_mode_get(sanadev_handle_t *sh, UWORD *mode)
{
  UBYTE id = 0;
  int res = plipbox_req_param_find_tag(sh, PARAM_TAG_MODE, &id);
  if (res != REQ_OK)
  {
    return res;
  }
  return plipbox_req_param_get_word(sh, id, mode);
}

// ncap

int param_tag_ncap_set(sanadev_handle_t *sh, UWORD flag)
{
  UBYTE id = 0;
  int res = plipbox_req_param_find_tag(sh, PARAM_TAG_NCAP, &id);
  if (res != REQ_OK)
  {
    return res;
  }
  return plipbox_req_param_set_word(sh, id, flag);
}

int param_tag_ncap_get(sanadev_handle_t *sh, UWORD *flag)
{
  UBYTE id = 0;
  int res = plipbox_req_param_find_tag(sh, PARAM_TAG_NCAP, &id);
  if (res != REQ_OK)
  {
    return res;
  }
  return plipbox_req_param_get_word(sh, id, flag);
}
