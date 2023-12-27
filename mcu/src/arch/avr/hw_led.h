#ifndef HW_LED_H
#define HW_LED_H

#include <avr/io.h>

#include "arch.h"
#include "led_pins.h"

extern void hw_led_init(void);
extern void hw_led_exit(void);
extern void hw_led_set(u08 on);

FORCE_INLINE void hw_led_on(void)
{
  LED_PORT |= LED_MASK;
}

FORCE_INLINE void hw_led_off(void)
{
  LED_PORT &= ~LED_MASK;
}

#endif
