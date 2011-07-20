#ifndef PAR_LOW_H
#define PAR_LOW_H

#include "global.h"

#include <avr/io.h>

/*
    Parallel Port Connection
                      AVR
    DATA 0 ... 5     PC 0 ... 5     IN/OUT
    DATA 6 ... 7     PD 6 ... 7     IN/OUT
    
    /STROBE          PD 2           IN (INT0)
    SELECT           PD 3           IN (INT1)
    BUSY             PD 4           OUT
    POUT             PD 5           IN
    /ACK             PB 0           OUT
*/

// lower bits of data
#define PAR_DATA_LO_MASK        0x3f
#define PAR_DATA_LO_PORT        PORTC
#define PAR_DATA_LO_PIN         PINC
#define PAR_DATA_LO_DDR         DDRC    

// high bits of data
#define PAR_DATA_HI_MASK        0xc0
#define PAR_DATA_HI_PORT        PORTD
#define PAR_DATA_HI_PIN         PIND
#define PAR_DATA_HI_DDR         DDRD

// /STROBE (IN) (INT0)
#define PAR_STROBE_BIT          2
#define PAR_STROBE_MASK         _BV(PAR_STROBE_BIT)
#define PAR_STROBE_PORT         PORTD
#define PAR_STROBE_PIN          PIND
#define PAR_STROBE_DDR          DDRD

// SELECT (IN) (INT1)
#define PAR_SELECT_BIT          3
#define PAR_SELECT_MASK         _BV(PAR_SELECT_BIT)
#define PAR_SELECT_PORT         PORTD
#define PAR_SELECT_PIN          PIND
#define PAR_SELECT_DDR          DDRD 

// BUSY (OUT)
#define PAR_BUSY_BIT            4
#define PAR_BUSY_MASK           _BV(PAR_BUSY_BIT)
#define PAR_BUSY_PORT           PORTD
#define PAR_BUSY_PIN            PIND
#define PAR_BUSY_DDR            DDRD 

// POUT (IN)
#define PAR_POUT_BIT            5
#define PAR_POUT_MASK           _BV(PAR_POUT_BIT)
#define PAR_POUT_PORT           PORTD
#define PAR_POUT_PIN            PIND
#define PAR_POUT_DDR            DDRD 

// /ACK (OUT)
#define PAR_ACK_BIT             0
#define PAR_ACK_MASK            _BV(PAR_ACK_BIT)
#define PAR_ACK_PORT            PORTB
#define PAR_ACK_PIN             PINB
#define PAR_ACK_DDR             DDRB

// ----- Flags -----

extern volatile u08 par_strobe_flag;
extern volatile u08 par_strobe_data;

// ----- Functions -----

extern void par_low_init(void);

// ----- Data Bus -----

extern void par_low_data_set_output(void);
extern void par_low_data_set_input(void);

inline void par_low_data_out(u08 d)
{
  PAR_DATA_LO_PORT &= ~PAR_DATA_LO_MASK;
  PAR_DATA_LO_PORT |= d & PAR_DATA_LO_MASK;
  PAR_DATA_HI_PORT &= ~PAR_DATA_HI_MASK;
  PAR_DATA_HI_PORT |= d & PAR_DATA_HI_MASK;
}

inline u08 par_low_data_in(void)
{
  u08 d1 = PAR_DATA_LO_PIN & PAR_DATA_LO_MASK;
  u08 d2 = PAR_DATA_HI_PIN & PAR_DATA_HI_MASK;
  return d1 | d2;
}

// ----- Signals -----

// /ACK (OUT)

inline void par_low_set_ack_lo(void)
{
  PAR_ACK_PORT &= ~PAR_ACK_MASK;
}

inline void par_low_set_ack_hi(void)
{
  PAR_ACK_PORT |= PAR_ACK_MASK;
}

// BUSY (OUT)

inline void par_low_set_busy_lo(void)
{
  PAR_BUSY_PORT &= ~PAR_BUSY_MASK;
}

inline void par_low_set_busy_hi(void)
{
  PAR_BUSY_PORT |= PAR_BUSY_MASK;
}

// STROBE (IN)

inline u08 par_low_get_strobe(void)
{
  return (PAR_STROBE_PIN & PAR_STROBE_MASK) == PAR_STROBE_MASK;
}

// SELECT (IN)

inline u08 par_low_get_select(void)
{
  return (PAR_SELECT_PIN & PAR_SELECT_MASK) == PAR_SELECT_MASK;
}

#endif