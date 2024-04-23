#ifndef MOD_CMD_H
#define MOD_CMD_H

#include "types.h"
#include "proto_status_shared.h"

extern void mode_cmd_init(void);

extern void mode_cmd_add_event_mask(u16 mask);
extern void mode_cmd_set_event_mask(u16 mask);
extern void mode_cmd_process_trigger(void);
extern u16  mode_cmd_take_event_mask(void);

extern void mode_cmd_set_rx_pending(void);

extern void mode_cmd_set_link_status(u16 status);
extern u16  mode_cmd_get_link_status(void);

extern void mode_cmd_set_hw_status(u16 status);
extern u16  mode_cmd_get_hw_status(void);

extern void mode_cmd_set_req_event_mask(u16 req_mask);
extern void mode_cmd_add_req_event_mask(u16 req_mask);

extern void mode_cmd_set_rx_error(u16 error);
extern void mode_cmd_set_rx_drop_count(u16 drops);
extern void mode_cmd_add_rx_drop_count(u16 drops);

extern void mode_cmd_set_tx_error(u16 error);
extern void mode_cmd_set_tx_drop_count(u16 drops);
extern void mode_cmd_add_tx_drop_count(u16 drops);

#endif
