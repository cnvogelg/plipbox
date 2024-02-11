#ifndef PLIPBOX_CMD_H
#define PLIPBOX_CMD_H

#include "sanadev.h"

/* special plipbox SANA-II commands */
BOOL plipbox_cmd_get_version(sanadev_handle_t *sh, UWORD *dev_version, UWORD *fw_version);
/* params */
BOOL plipbox_cmd_param_get_num(sanadev_handle_t *sh, UWORD *num);
BOOL plipbox_cmd_param_find_tag(sanadev_handle_t *sh, ULONG tag, UWORD *id);
BOOL plipbox_cmd_param_get_def(sanadev_handle_t *sh, UWORD id, s2pb_param_def_t *def);
BOOL plipbox_cmd_param_get_val(sanadev_handle_t *sh, UWORD id, UWORD size, UBYTE *data);
BOOL plipbox_cmd_param_set_val(sanadev_handle_t *sh, UWORD id, UWORD size, UBYTE *data);

BOOL plipbox_cmd_param_get_word(sanadev_handle_t *sh, UWORD id, UWORD *value);
BOOL plipbox_cmd_param_set_word(sanadev_handle_t *sh, UWORD id, UWORD value);

/* prefs */
BOOL plipbox_cmd_prefs_reset(sanadev_handle_t *sh);
BOOL plipbox_cmd_prefs_load(sanadev_handle_t *sh, UWORD *status);
BOOL plipbox_cmd_prefs_save(sanadev_handle_t *sh, UWORD *status);

#endif
