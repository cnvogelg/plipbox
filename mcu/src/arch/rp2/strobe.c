#include "pico/stdlib.h"
#include "arch.h"

#include "autoconf.h"
#include "types.h"
#include "strobe.h"
#include "pario_pins.h"
#include "hw_timer.h"

static volatile u08 state;
static u08 strobe_count;
static volatile u32 strobe_key;

static strobe_send_get_func_t send_get_func;
static u16 send_size;

typedef void (*strobe_func_t)(void);

static strobe_func_t strobe_func;

static void strobe_callback(uint gpio, uint32_t events)
{
  if((gpio == PAR_STROBE_PIN) && (events & GPIO_IRQ_EDGE_FALL)) {
    strobe_func();
  }
}

static void strobe_read_func(void)
{
  u08 data = pario_get_data();

  // readable char?
  if((data >= 32) && (data < 128)) {
    strobe_key <<= 8;
    strobe_key |= data;
    strobe_count++;
    if(strobe_count == 4) {
      state = 1;
      strobe_count = 0;
    }
  }

  // pulse ack
  pario_ack_lo();
  hw_timer_delay_1us();
  pario_ack_hi();
}

void strobe_init_port(void)
{
  pario_init();
}

void strobe_init_irq(void)
{
  strobe_key = 0;
  strobe_count = 0;
  state = 0;

  strobe_func = strobe_read_func;

  gpio_set_irq_enabled_with_callback(PAR_STROBE_PIN, GPIO_IRQ_EDGE_FALL, 
    true, strobe_callback);
}

void strobe_exit(void)
{
  gpio_set_irq_enabled(PAR_STROBE_PIN, GPIO_IRQ_EDGE_FALL, false);
}

u08 strobe_get_key(u32 *key)
{
  // return with exit byte?
  u08 exit = 0;

  irq_off();

  if(state != 0) {
    *key = strobe_key;
    state = 0;
    exit = 1;
  }

  irq_on();

  return exit;
}

u08 strobe_get_data(void)
{
  return pario_get_data();
}

// ----- send -----

static void strobe_write_func(void)
{
  // nothing more to send...
  if(send_size == 0) {
    pario_set_data(0);
    state = STROBE_FLAG_GOT_STROBE | STROBE_FLAG_ALL_SENT;
  } else {
    state = STROBE_FLAG_GOT_STROBE;

    // setup next byte
    u08 data = send_get_func();
    pario_set_data(data);
    send_size--;
  }

  // 0xc4 is AmigaDOS buffer size for type command
  strobe_count++;
  if(strobe_count == 0xc4) {
    strobe_count = 0;
    // suppress ack if ended
    if(send_size == 0) {
      state |= STROBE_FLAG_BUFFER_FILLED;
      return;
    }
  }

  // pulse ack
  pario_ack_lo();
  hw_timer_delay_1us();
  pario_ack_hi();
}

void strobe_send_begin(strobe_send_get_func_t func, u16 size)
{
  // keep send range
  send_get_func = func;
  send_size = size;

  // setup first byte
  u08 val = send_get_func();
  send_size--;

  strobe_func = strobe_write_func;

  pario_busy_in();
  pario_set_data(val);
  pario_data_ddr_out();

  state = STROBE_FLAG_NONE;
  strobe_count = 0;
}

u08 strobe_read_flag(void)
{
  // read busy
  u08 res = pario_get_busy() ? STROBE_FLAG_IS_BUSY : STROBE_FLAG_NONE;

  irq_off();

  res |= state;
  state = STROBE_FLAG_NONE;

  irq_on();

  return res;
}

void strobe_pulse_ack(void)
{
  // pulse ack
  pario_ack_lo();
  hw_timer_delay_1us();
  pario_ack_hi();
}

void strobe_send_end(void)
{
  pario_data_ddr_in();
  pario_busy_out();

  strobe_func = strobe_read_func;

  state = 0;
  strobe_count = 0;
}
