#ifndef PARAM_TAG_H
#define PARAM_TAG_H

#include "sanadev.h"

extern BOOL param_tag_mode_set(sanadev_handle_t *sh, UWORD mode);
extern BOOL param_tag_mode_get(sanadev_handle_t *sh, UWORD *mode);

extern BOOL param_tag_ncap_set(sanadev_handle_t *sh, UWORD mode);
extern BOOL param_tag_ncap_get(sanadev_handle_t *sh, UWORD *mode);

#endif
