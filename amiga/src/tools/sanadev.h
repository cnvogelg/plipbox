#ifndef SANADEV_H
#define SANADEV_H

#include <devices/plipbox.h>

struct sanadev_handle;
typedef struct sanadev_handle sanadev_handle_t;

#define SANADEV_MAC_SIZE 6
typedef UBYTE sanadev_mac_t[SANADEV_MAC_SIZE];

#define SANADEV_ETH_MIN_FRAME_SIZE 64
#define SANADEV_ETH_MAX_FRAME_SIZE 1500
#define SANADEV_ETH_RAW_FRAME_SIZE 1514

#define SANADEV_OK 0
#define SANADEV_ERROR_NO_PORT 1
#define SANADEV_ERROR_NO_IOREQ 2
#define SANADEV_ERROR_OPEN_DEVICE 3

/* API */
extern sanadev_handle_t *sanadev_open(const char *name, ULONG unit, ULONG flags, UWORD *error);
extern void sanadev_close(sanadev_handle_t *sh);

/* regular SANA-II commands */
BOOL sanadev_cmd_online(sanadev_handle_t *sh);
BOOL sanadev_cmd_offline(sanadev_handle_t *sh);
BOOL sanadev_cmd_get_station_address(sanadev_handle_t *sh, sanadev_mac_t cur_mac, sanadev_mac_t def_mac);

/* custom commands */
struct IOSana2Req *sanadev_cmd_req(sanadev_handle_t *sh);
BOOL sanadev_cmd(sanadev_handle_t *sh, UWORD cmd);

/* event handling */
BOOL sanadev_event_init(sanadev_handle_t *sh, UWORD *error);
void sanadev_event_exit(sanadev_handle_t *sh);
BOOL sanadev_event_start(sanadev_handle_t *sh, ULONG event_mask);
BOOL sanadev_event_stop(sanadev_handle_t *sh);
ULONG sanadev_event_get_mask(sanadev_handle_t *sh);
BOOL sanadev_event_result(sanadev_handle_t *sh, ULONG *event_mask);
BOOL sanadev_event_wait(sanadev_handle_t *sh, ULONG *event_mask);

/* io: read/write ops */
BOOL sanadev_io_init(sanadev_handle_t *sh, UWORD *error);
void sanadev_io_exit(sanadev_handle_t *sh);
/* io: sync tx */
BOOL sanadev_io_write(sanadev_handle_t *sh, UWORD pkt_type, sanadev_mac_t dst_addr, APTR data, ULONG data_len);
BOOL sanadev_io_write_raw(sanadev_handle_t *sh, APTR data, ULONG data_len);
BOOL sanadev_io_broadcast(sanadev_handle_t *sh, UWORD pkt_type, APTR data, ULONG data_len);
/* io: async rx */
BOOL sanadev_io_read_start(sanadev_handle_t *sh, UWORD pkt_type, APTR data, ULONG data_len, BOOL raw);
BOOL sanadev_io_read_start_orphan(sanadev_handle_t *sh, APTR data, ULONG data_len, BOOL raw);
BOOL sanadev_io_read_stop(sanadev_handle_t *sh);
ULONG sanadev_io_read_get_mask(sanadev_handle_t *sh);
BOOL sanadev_io_read_result(sanadev_handle_t *sh, UWORD *pkt_type, sanadev_mac_t dst_addr, UBYTE **data, ULONG *data_len);
BOOL sanadev_io_read_result_raw(sanadev_handle_t *sh, UBYTE **data, ULONG *data_len);

/* helper */
void sanadev_cmd_get_error(sanadev_handle_t *sh, BYTE *error, ULONG *wire_error);
void sanadev_cmd_print_error(sanadev_handle_t *sh);

void sanadev_io_write_get_error(sanadev_handle_t *sh, BYTE *error, ULONG *wire_error);
void sanadev_io_write_print_error(sanadev_handle_t *sh);

void sanadev_io_read_get_error(sanadev_handle_t *sh, BYTE *error, ULONG *wire_error);
void sanadev_io_read_print_error(sanadev_handle_t *sh);

char *sanadev_error_string(BYTE error);
char *sanadev_wire_error_string(ULONG wire_error);

void sanadev_print_mac(sanadev_mac_t mac);

#endif
