#ifndef PROTO_LOW_H
#define PROTO_LOW_H

extern void proto_low_init(void);
extern u08 proto_low_get_cmd(void);

extern void proto_low_action(void);
extern void proto_low_end(void);

extern void proto_low_read_word(u16 v);
extern u16  proto_low_write_word(void);

extern void proto_low_read_long(u32 v);
extern u32  proto_low_write_long(void);

extern void proto_low_write_block(u16 num_words, u08 *buffer);
extern void proto_low_read_block(u16 num_words, u08 *buffer);

extern void proto_low_write_block_spi(u16 num_words);
extern void proto_low_read_block_spi(u16 num_words);

extern void proto_low_ack_lo(void);
extern void proto_low_ack_hi(void);

extern void proto_low_busy_lo(void);
extern void proto_low_busy_hi(void);

#endif
