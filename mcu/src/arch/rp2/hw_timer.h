#ifndef HW_TIMER_H
#define HW_TIMER_H

#include "pico/time.h"
#include "hardware/timer.h"
#include "arch.h"

typedef uint32_t hw_timer_ms_t;

INLINE void hw_timer_init(void)
{
}

INLINE hw_timer_ms_t hw_timer_millis(void)
{
    return to_ms_since_boot(get_absolute_time());
}

INLINE int hw_timer_millis_timed_out(hw_timer_ms_t start, uint16_t timeout)
{
    return (hw_timer_millis() - start) > timeout;
}

FORCE_INLINE void hw_timer_delay_ms(uint32_t ms)
{
    busy_wait_ms(ms);
}

FORCE_INLINE void hw_timer_delay_us(uint32_t usec)
{
    busy_wait_us_32(usec);
}

#define hw_timer_delay_1us() hw_timer_delay_us(1)

#endif
