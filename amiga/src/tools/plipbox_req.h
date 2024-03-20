#ifndef PLIPBOX_REQ_H
#define PLIPBOX_REQ_H

#include "sanadev.h"
#include "param.h"
#include "req_shared.h"

/* params */
int  plipbox_req_param_get_num(sanadev_handle_t *sh, UBYTE *num);
int  plipbox_req_param_find_tag(sanadev_handle_t *sh, ULONG tag, UBYTE *id);
int  plipbox_req_param_get_def(sanadev_handle_t *sh, UBYTE id, param_def_t *def);
int  plipbox_req_param_get_val(sanadev_handle_t *sh, UBYTE id, UWORD size, UBYTE *data);
int  plipbox_req_param_set_val(sanadev_handle_t *sh, UBYTE id, UWORD size, UBYTE *data);

int  plipbox_req_param_get_word(sanadev_handle_t *sh, UBYTE id, UWORD *value);
int  plipbox_req_param_set_word(sanadev_handle_t *sh, UBYTE id, UWORD value);

/* prefs */
int  plipbox_req_prefs_reset(sanadev_handle_t *sh);
int  plipbox_req_prefs_load(sanadev_handle_t *sh);
int  plipbox_req_prefs_save(sanadev_handle_t *sh);

#endif
