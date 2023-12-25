#ifndef PROTO_ATOM_H
#define PROTO_ATOM_H

#define PROTO_NO_CMD      0xff

extern void proto_atom_init(void);

extern void proto_atom_pulse_irq(void);
extern void proto_atom_set_busy(void);
extern void proto_atom_clr_busy(void);

extern void proto_atom_action(void);
extern void proto_atom_read_word(u16 val);
extern u16  proto_atom_write_word(void);
extern void proto_atom_read_long(u32 val);
extern u32  proto_atom_write_long(void);

extern void proto_atom_read_block(u08 *buf, u16 num_bytes);
extern void proto_atom_write_block(u08 *buf, u16 num_bytes);

extern void proto_atom_read_block_nospi(u08 *buf, u16 num_bytes);
extern void proto_atom_write_block_nospi(u08 *buf, u16 num_bytes);

extern u08 proto_atom_get_cmd(void);

#endif
