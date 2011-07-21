#include "par_low.h"
#include <avr/interrupt.h>
#include <util/delay_basic.h>

volatile u08 par_low_strobe_flag = 0;

void par_low_init(void)
{
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
  EIFR = 0;
  EIMSK = _BV(INT0);
  sei();

  par_low_data_set_input();
}

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
  
  par_low_strobe_flag = 0;
}

void par_low_pulse_ack(void)
{
  par_low_set_ack_lo();
  _delay_loop_1((F_CPU + 2999999) / 3000000); // approx 1us if delay approx 3 instr
  par_low_set_ack_hi();
}

// INT0 Strobe Handler
ISR(INT0_vect)
{
  par_low_strobe_flag = 1;
}
