#ifndef HW_LED_H
#define HW_LED_H

#include "hardware/gpio.h"
#include "arch.h"

extern void hw_led_init(void);
extern void hw_led_exit(void);
extern void hw_led_set(int on);

INLINE void hw_led_on(void)
{
  gpio_put(PICO_DEFAULT_LED_PIN, true);
}

INLINE void hw_led_off(void)
{
  gpio_put(PICO_DEFAULT_LED_PIN, false);
}

#endif
