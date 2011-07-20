#include "par_low.h"
#include <avr/interrupt.h>

void par_low_init(void)
{
  par_low_data_set_input();

  // /STROBE (IN)
  PAR_STROBE_DDR &= ~PAR_STROBE_MASK;
  PAR_STROBE_PORT |= PAR_STROBE_MASK;
  
  // SELECT (IN)
  PAR_SELECT_DDR &= ~PAR_SELECT_MASK;
  PAR_SELECT_PORT |= PAR_SELECT_MASK;
  
  // BUSY (OUT) (0)
  PAR_BUSY_DDR |= PAR_BUSY_MASK;
  PAR_BUSY_PORT &= ~PAR_BUSY_MASK;
  
  // POUT (IN)
  PAR_POUT_DDR &= ~PAR_POUT_MASK;
  PAR_POUT_PORT |= PAR_POUT_MASK;
  
  // /ACK (OUT) (1)
  PAR_ACK_DDR |= PAR_ACK_MASK;
  PAR_ACK_PORT |= PAR_ACK_MASK;
  
  // setup STROBE/SELECT IRQ
  cli();
  EICRA = _BV(ISC01); // falling edge of INT0 (STROBE)
  EIMSK = _BV(INT0);
  sei();
}

// input buffer 

volatile u08 par_in_buf[PAR_IN_BUF_SIZE];
volatile u08 par_in_put = 0;
volatile u08 par_in_get = 0;
volatile u08 par_in_overflows = 0;

// data bus

void par_low_data_set_output(void)
{
  PAR_DATA_LO_DDR |= PAR_DATA_LO_MASK;
  PAR_DATA_HI_DDR |= PAR_DATA_HI_MASK;
}

void par_low_data_set_input(void)
{
  PAR_DATA_LO_DDR &= ~PAR_DATA_LO_MASK;
  PAR_DATA_HI_DDR &= ~PAR_DATA_HI_MASK;
}

// INT0 Strobe Handler
ISR(INT0_vect)
{
  u08 pos = par_in_put;
  // read value into buffer
  par_in_buf[pos] = par_low_data_in();
  
  // next pos
  pos = (pos + 1) & PAR_IN_BUF_MASK;
  par_in_put = pos;
  
  // check for overflows
  if(pos == par_in_get) {
    par_in_overflows++;
  }
  
  // signal receiption -> toggle HS_LINE (aka BUSY)
  par_low_toggle_busy();
}
