
#include <avr/wdt.h>
#include <avr/interrupt.h>

#include "hw_timer.h"

void hw_system_init(void)
{
#ifdef MACH_TEENSY20
  // set teensy clock to 16 MHz
#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
#define CPU_16MHz       0x00
  CPU_PRESCALE(CPU_16MHz);
#endif
}

void hw_system_reset(void)
{
  wdt_enable(WDTO_15MS);
  while(1) { /* wait for the end */ }
}
