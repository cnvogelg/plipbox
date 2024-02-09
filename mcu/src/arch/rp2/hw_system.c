#include "hardware/watchdog.h"

#include "hw_system.h"

void hw_system_init(void)
{
}

void hw_system_reset(void)
{
  watchdog_enable(1, true);
  // wait for my death
  while(1) {}
}
