#ifndef RX_BUF_H
#define RX_BUF_H

#ifndef CONFIG_RX_BUF_NUM
#define CONFIG_RX_BUF_NUM 4
#endif

extern void rx_buf_init(void);

extern u08  rx_buf_capacity(void);
extern u08  rx_buf_size(void);
extern u08  rx_buf_free(void);

extern u08 *rx_buf_put_begin(u16 size);
extern void rx_buf_put_end(void);

extern u08 *rx_buf_get_begin(u16 *size);
extern void rx_buf_get_end(void);

extern u16  rx_buf_peek_buf_size(void);

#endif
