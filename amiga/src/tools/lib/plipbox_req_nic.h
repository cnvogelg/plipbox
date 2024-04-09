#ifndef PLIPBOX_REQ_NIC_H
#define PLIPBOX_REQ_NIC_H

#include "sanadev.h"
#include "nic.h"
#include "req_shared.h"

/* nic reqs */
int  plipbox_req_nic_get_num(sanadev_handle_t *sh, UBYTE *num);
int  plipbox_req_nic_find_tag(sanadev_handle_t *sh, ULONG tag, UBYTE *id);
int  plipbox_req_nic_get_def(sanadev_handle_t *sh, UBYTE id, nic_def_t *def);

#endif
