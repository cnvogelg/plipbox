#include "hw_led.h"

void hw_led_init(void)
{
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

void hw_led_exit(void)
{
}

void hw_led_set(int on)
{
  gpio_put(PICO_DEFAULT_LED_PIN, on);
}
