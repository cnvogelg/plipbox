#include "hardware/gpio.h"

#include "autoconf.h"
#include "types.h"

#include "hw_dev.h"
#include "hw_timer.h"
#include "dev_pins.h"

void hw_dev_init(void)
{
  gpio_init(DEV_RESET_PIN);
  gpio_set_dir(DEV_RESET_PIN, GPIO_OUT);
  gpio_put(DEV_RESET_PIN, true);
}

void hw_dev_reset(void)
{
  gpio_put(DEV_RESET_PIN, false);
  hw_timer_delay_us(2);
  gpio_put(DEV_RESET_PIN, true);
}
