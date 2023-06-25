#ifndef HW_TIMER_H
#define HW_TIMER_H

#include <avr/io.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <stdint.h>

#include "arch.h"
#include "types.h"

// unit of my timers
typedef u16 hw_timer_ms_t;

extern volatile hw_timer_ms_t  timer_ms;


extern void hw_timer_init(void);

#define hw_timer_delay_ms(ms) _delay_ms(ms)
#define hw_timer_delay_us(us) _delay_us(us)
#define hw_timer_delay_1us()  _delay_loop_1(F_CPU / 1000000)

INLINE hw_timer_ms_t hw_timer_millis(void)
{
  hw_timer_ms_t val;
  ATOMIC_BLOCK(ATOMIC_FORCEON)
  {
    val = timer_ms;
  }
  return val;
}

INLINE u08 hw_timer_millis_timed_out(hw_timer_ms_t start, u16 timeout)
{
  hw_timer_ms_t cur = hw_timer_millis();
  if(cur >= start) {
    return (cur - start) > timeout;
  } else {
    return (cur + (~start)) > timeout;
  }
}

#endif
