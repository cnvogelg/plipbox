#include "global.h"

#ifdef DEBUG_PROTO_ATOM
#define DEBUG
#endif

#include "proto_low.h"
#include "proto_atom.h"
#include "debug.h"
#include "hw_timer.h"

void proto_atom_init(void)
{
  proto_low_init();
  DC('I');
}

void proto_atom_pulse_irq(void)
{
  // trigger ack irq at Amiga
  DC('#');
  proto_low_ack_lo();
  hw_timer_delay_1us();
  proto_low_ack_hi();
}

void proto_atom_set_busy(void)
{
  DC('B');
  proto_low_busy_hi();
}

void proto_atom_clr_busy(void)
{
  DC('b');
  proto_low_busy_lo();
}

void proto_atom_action(void)
{
  DC('A');
  proto_low_action();
  proto_low_end();
}

void proto_atom_read_word(u16 val)
{
  DC('r'); DC('w');
  proto_low_read_word(val);
  proto_low_end();
}

u16 proto_atom_write_word(void)
{
  u16 val = proto_low_write_word();
  proto_low_end();
  return val;
}

void proto_atom_read_long(u32 val)
{
  proto_low_read_long(val);
  proto_low_end();
}

u32 proto_atom_write_long(void)
{
  u32 val = proto_low_write_long();
  proto_low_end();
  return val;
}

void proto_atom_read_block(u08 *buf, u16 num_bytes)
{
  u16 num_words = num_bytes >> 1;
  if(buf == NULL) {
    DC('#');
    proto_low_read_block_spi(num_words);
  } else {
    proto_low_read_block(num_words, buf);
  }
  proto_low_end();
}

void proto_atom_write_block(u08 *buf, u16 num_bytes)
{
  u16 num_words = num_bytes >> 1;
  if(buf == NULL) {
    DC('#');
    proto_low_write_block_spi(num_words);
  } else {
    proto_low_write_block(num_words, buf);
  }
  proto_low_end();
}

void proto_atom_read_block_nospi(u08 *buf, u16 num_bytes)
{
  u16 num_words = num_bytes >> 1;
  proto_low_read_block(num_words, buf);
  proto_low_end();
}

void proto_atom_write_block_nospi(u08 *buf, u16 num_bytes)
{
  u16 num_words = num_bytes >> 1;
  proto_low_write_block(num_words, buf);
  proto_low_end();
}

u08 proto_atom_get_cmd(void)
{
  // read command from bits 0..4 in idle byte
  u08 cmd = proto_low_get_cmd();
  if((cmd == PROTO_NO_CMD) || (cmd==0)) {
    // no clock lined pulled -> idle
    return PROTO_NO_CMD;
  }

  DC('['); DB(cmd); DC(']'); DNL;
  return cmd;
}
