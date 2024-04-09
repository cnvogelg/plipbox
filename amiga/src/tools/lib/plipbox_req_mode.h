#ifndef PLIPBOX_REQ_MODE_H
#define PLIPBOX_REQ_MODE_H

#include "sanadev.h"
#include "mode.h"
#include "req_shared.h"

/* mode reqs */
int  plipbox_req_mode_get_num(sanadev_handle_t *sh, UBYTE *num);
int  plipbox_req_mode_find_tag(sanadev_handle_t *sh, ULONG tag, UBYTE *id);
int  plipbox_req_mode_get_def(sanadev_handle_t *sh, UBYTE id, mode_def_t *def);

#endif
