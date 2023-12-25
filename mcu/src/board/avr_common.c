#include <avr/wdt.h>

void board_reset(void)
{
  wdt_enable(WDTO_15MS);
  while(1) { /* wait for the end */ }
}
