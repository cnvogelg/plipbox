#ifndef SANADEV_H
#define SANADEV_H

struct sanadev_handle;
typedef struct sanadev_handle sanadev_handle_t;

#define SANADEV_MAC_SIZE 6
typedef UBYTE sanadev_mac_t[SANADEV_MAC_SIZE];

#define SANADEV_OK                  0
#define SANADEV_ERROR_NO_PORT       1
#define SANADEV_ERROR_NO_IOREQ      2
#define SANADEV_ERROR_OPEN_DEVICE   3

extern sanadev_handle_t *sanadev_open(const char *name, ULONG unit, ULONG flags, UWORD *error);
extern void sanadev_close(sanadev_handle_t *sh);

/* regular SANA-II commands */
BOOL sanadev_cmd_online(sanadev_handle_t *sh);
BOOL sanadev_cmd_offline(sanadev_handle_t *sh);
BOOL sanadev_cmd_get_station_address(sanadev_handle_t *sh, sanadev_mac_t cur_mac, sanadev_mac_t def_mac);

/* special plipbox commands */
BOOL sanadev_cmd_get_version(sanadev_handle_t *sh, UWORD *dev_version, UWORD *fw_version);
BOOL sanadev_cmd_plipbox_set_mac(sanadev_handle_t *sh, sanadev_mac_t new_mac);
BOOL sanadev_cmd_plipbox_set_mode(sanadev_handle_t *sh, UWORD mode);
BOOL sanadev_cmd_plipbox_get_mode(sanadev_handle_t *sh, UWORD *mode);

/* helper */
void sanadev_cmd_get_error(sanadev_handle_t *sh, UWORD *error, UWORD *wire_error);
void sanadev_cmd_print_error(sanadev_handle_t *sh);

void sanadev_print_mac(sanadev_mac_t mac);

#endif
