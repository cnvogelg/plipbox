#ifndef SANADEV_H
#define SANADEV_H

#include <devices/plipbox.h>

struct sanadev_handle;
typedef struct sanadev_handle sanadev_handle_t;

#define SANADEV_MAC_SIZE 6
typedef UBYTE sanadev_mac_t[SANADEV_MAC_SIZE];

#define SANADEV_OK                  0
#define SANADEV_ERROR_NO_PORT       1
#define SANADEV_ERROR_NO_IOREQ      2
#define SANADEV_ERROR_OPEN_DEVICE   3

/* API */
extern sanadev_handle_t *sanadev_open(const char *name, ULONG unit, ULONG flags, UWORD *error);
extern void sanadev_close(sanadev_handle_t *sh);

/* regular SANA-II commands */
BOOL sanadev_cmd_online(sanadev_handle_t *sh);
BOOL sanadev_cmd_offline(sanadev_handle_t *sh);
BOOL sanadev_cmd_get_station_address(sanadev_handle_t *sh, sanadev_mac_t cur_mac, sanadev_mac_t def_mac);

/* event handling */
BOOL  sanadev_event_init(sanadev_handle_t *sh, UWORD *error);
void  sanadev_event_exit(sanadev_handle_t *sh);
void  sanadev_event_start(sanadev_handle_t *sh, ULONG event_mask);
void  sanadev_event_stop(sanadev_handle_t *sh);
ULONG sanadev_event_get_mask(sanadev_handle_t *sh);
BOOL  sanadev_event_get_event(sanadev_handle_t *sh, ULONG *event_mask);
BOOL  sanadev_event_wait(sanadev_handle_t *sh, ULONG *event_mask);

/* special plipbox commands */
BOOL sanadev_cmd_plipbox_get_version(sanadev_handle_t *sh, UWORD *dev_version, UWORD *fw_version);
/* params */
BOOL sanadev_cmd_plipbox_param_get_num(sanadev_handle_t *sh, UWORD *num);
BOOL sanadev_cmd_plipbox_param_find_tag(sanadev_handle_t *sh, ULONG tag, UWORD *id);
BOOL sanadev_cmd_plipbox_param_get_def(sanadev_handle_t *sh, UWORD id, s2pb_param_def_t *def);
BOOL sanadev_cmd_plipbox_param_get_val(sanadev_handle_t *sh, UWORD id, UWORD size, UBYTE *data);
BOOL sanadev_cmd_plipbox_param_set_val(sanadev_handle_t *sh, UWORD id, UWORD size, UBYTE *data);
/* prefs */
BOOL sanadev_cmd_plipbox_prefs_reset(sanadev_handle_t *sh);
BOOL sanadev_cmd_plipbox_prefs_load(sanadev_handle_t *sh, UWORD *status);
BOOL sanadev_cmd_plipbox_prefs_save(sanadev_handle_t *sh, UWORD *status);

/* helper */
void sanadev_cmd_get_error(sanadev_handle_t *sh, UWORD *error, UWORD *wire_error);
void sanadev_cmd_print_error(sanadev_handle_t *sh);

void sanadev_print_mac(sanadev_mac_t mac);

#endif
