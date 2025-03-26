#ifndef PROTO_API_H
#define PROTO_API_H

extern void proto_api_init(void);

extern void proto_api_set_link_status(u08 status);
extern void proto_api_set_nic_status(u08 status);
extern u08  proto_api_get_link_status(void);
extern u08  proto_api_get_nic_status(void);

extern void proto_api_add_event_mask(u16 mask);
extern void proto_api_set_event_mask(u16 mask);
extern u16  proto_api_take_event_mask(void);
extern void proto_api_process_trigger(void);

extern void proto_api_set_rx_pending(void);

extern void proto_api_set_req_event_mask(u16 req_mask);
extern void proto_api_add_req_event_mask(u16 req_mask);

extern void proto_api_set_rx_error(u16 error);
extern void proto_api_set_rx_drop_count(u16 drops);
extern void proto_api_add_rx_drop_count(u16 drops);

extern void proto_api_set_tx_error(u16 error);
extern void proto_api_set_tx_drop_count(u16 drops);
extern void proto_api_add_tx_drop_count(u16 drops);

// ----- command calls by proto -----
extern void proto_api_cmd_ping(void);
extern u16  proto_api_cmd_get_version(void);

extern u16  proto_api_cmd_mode_get(void);
extern void proto_api_cmd_mode_set(u16 mode);
extern u16  proto_api_cmd_mode_attach(void);
extern u16  proto_api_cmd_mode_detach(void);

extern u16  proto_api_cmd_event_mask(void);
extern u16  proto_api_cmd_nic_status(void);
extern u16  proto_api_cmd_link_status(void);
extern u16  proto_api_cmd_mode_status(void);

extern u16  proto_api_cmd_rx_size(void);
extern u08 *proto_api_cmd_rx_begin(u16 size);
extern u16  proto_api_cmd_rx_end(u16 size);
extern u16  proto_api_cmd_rx_error(void);
extern u16  proto_api_cmd_rx_drop_count(void);

extern u08 *proto_api_cmd_tx_begin(u16 size);
extern u16  proto_api_cmd_tx_end(u16 size);
extern u16  proto_api_cmd_tx_error(void);
extern u16  proto_api_cmd_tx_drop_count(void);

extern u16  proto_api_cmd_req_event_mask(void);

// ----- request -----
struct proto_api_req {
  u08 command;
  u08 in_extra;
  u08 status;
  u08 out_extra;
  u16 in_size;
  u16 out_size;
  u08 *in_buf;
  u08 *out_buf;
};
typedef struct proto_api_req proto_api_req_t;

extern void proto_api_cmd_req_in(proto_api_req_t *req);
extern void proto_api_cmd_req_out(proto_api_req_t *req);

#endif
