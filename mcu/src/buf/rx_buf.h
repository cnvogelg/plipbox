#ifndef RX_BUF_H
#define RX_BUF_H

#ifndef CONFIG_RX_BUF_SIZE_KB
#define CONFIG_RX_BUF_SIZE_KB 8
#endif

extern void rx_buf_init(void);

extern u16  rx_buf_get_size(void);
extern u16  rx_buf_get_used(void);
extern u16  rx_buf_get_free(void);
extern u16  rx_buf_get_num_pkt(void);

extern u08  rx_buf_put(const u08 *data, u16 size);
extern u16  rx_buf_get(u08 *data);

extern u16  rx_buf_peek_buf_size(void);

extern void rx_buf_dump(void);

#endif
