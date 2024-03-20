#ifndef PLIPBOX_CMD_H
#define PLIPBOX_CMD_H

#include "sanadev.h"

/* special plipbox SANA-II commands */
BOOL plipbox_cmd_get_version(sanadev_handle_t *sh, UWORD *dev_version, UWORD *fw_version);
int  plipbox_cmd_do_request(sanadev_handle_t *sh, s2pb_request_t *req);

#endif
