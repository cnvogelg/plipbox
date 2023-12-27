#include "autoconf.h"
#include "types.h"

#include "pario_pins.h"
#include "proto_low.h"
#include "hw_spi.h"

/* signal names:
   IN
      SELECT -> clk
      STROBE -> strobe
   OUT
      ACK -> signal
      POUT -> rak
*/

#define ddr_in()  pario_data_ddr_in()
#define ddr_out() pario_data_ddr_out()
#define dout(x)   pario_set_data(x)
#define din()     pario_get_data()

#define clk()     pario_get_select()
#define strobe()  pario_get_strobe()

#define rak_lo()  pario_pout_lo()
#define rak_hi()  pario_pout_hi()
#define ack_lo()  pario_ack_lo()
#define ack_hi()  pario_ack_hi()
#define busy_lo() pario_busy_lo()
#define busy_hi() pario_busy_hi()

#define wait_clk_hi()  while(!clk()) {}
#define wait_clk_lo()  while(clk()) {}

void proto_low_init(void)
{
    pario_init();
}

u08 proto_low_get_cmd(void)
{
  // clock is low -> no command
  if(clk()) {
    return 0xff;
  }

  // read data (command nybble)
  return din();
}

void FAST_FUNC(proto_low_action)(void)
{
  rak_lo();
}

void FAST_FUNC(proto_low_end)(void)
{
  wait_clk_hi();
  rak_hi();
}

void FAST_FUNC(proto_low_read_word)(u16 v)
{
  u08 a = (u08)(v >> 8);
  u08 b = (u08)(v & 0xff);

  irq_off();

  rak_lo();
  wait_clk_hi();
  ddr_out();

  wait_clk_lo();
  dout(a);
  wait_clk_hi();
  dout(b);

  wait_clk_lo();
  ddr_in();

  irq_on();
}

u16 FAST_FUNC(proto_low_write_word)(void)
{
  irq_off();

  rak_lo();

  wait_clk_hi();
  u08 a = din();
  wait_clk_lo();
  u08 b = din();

  irq_on();
  return (a << 8) | b;
}

void FAST_FUNC(proto_low_read_long)(u32 v)
{
  u08 a = (u08)(v >> 24);
  u08 b = (u08)(v >> 16);
  u08 c = (u08)(v >> 8);
  u08 d = (u08)(v & 0xff);

  irq_off();

  rak_lo();
  wait_clk_hi();
  ddr_out();

  wait_clk_lo();
  dout(a);
  wait_clk_hi();
  dout(b);
  wait_clk_lo();
  dout(c);
  wait_clk_hi();
  dout(d);

  wait_clk_lo();
  ddr_in();

  irq_on();
}

u32 FAST_FUNC(proto_low_write_long)(void)
{
  irq_off();

  rak_lo();

  wait_clk_hi();
  u08 a = din();
  wait_clk_lo();
  u08 b = din();
  wait_clk_hi();
  u08 c = din();
  wait_clk_lo();
  u08 d = din();

  irq_on();
  return (a << 24) | (b << 16) | (c << 8) | d;
}

void FAST_FUNC(proto_low_write_block)(u16 max_words, u08 *buffer)
{
  irq_off();

  rak_lo();

  for(u16 i=0;i<max_words;i++) {
    wait_clk_hi();
    *buffer++ = din();
    wait_clk_lo();
    *buffer++ = din();
  }

  irq_on();
}

void FAST_FUNC(proto_low_read_block)(u16 num_words, u08 *buffer)
{
  irq_off();

  rak_lo();
  wait_clk_hi();
  ddr_out();

  for(u16 i=0;i<num_words;i++) {
    wait_clk_lo();
    dout(*buffer++);
    wait_clk_hi();
    dout(*buffer++);
  }

  wait_clk_lo();
  ddr_in();

  irq_on();
}

void FAST_FUNC(proto_low_write_block_spi)(u16 max_words)
{
  irq_off();

  rak_lo();

  for(u16 i=0;i<max_words;i++) {
    wait_clk_hi();
    u08 d = din();
    hw_spi_out(d);

    wait_clk_lo();
    d = din();
    hw_spi_out(d);
  }

  irq_on();
}

void FAST_FUNC(proto_low_read_block_spi)(u16 num_words)
{
  irq_off();

  rak_lo();
  wait_clk_hi();
  ddr_out();

  for(u16 i=0;i<num_words;i++) {
    u08 d = hw_spi_in();
    wait_clk_lo();
    dout(d);

    d = hw_spi_in();
    wait_clk_hi();
    dout(d);
  }

  wait_clk_lo();
  ddr_in();

  irq_on();
}

void proto_low_ack_lo(void)
{
  ack_lo();
}

void proto_low_ack_hi(void)
{
  ack_hi();
}

void proto_low_busy_lo(void)
{
  busy_lo();
}

void proto_low_busy_hi(void)
{
  busy_hi();
}
