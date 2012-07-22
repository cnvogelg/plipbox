/*
 * par_low.h - low_level routines to access amiga parallel port
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plipbox.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

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

// ----- Input Buffer Handling -----

#define PAR_IN_BUF_BITS     4
#define PAR_IN_BUF_SIZE     (1 << PAR_IN_BUF_BITS)
#define PAR_IN_BUF_MASK     (PAR_IN_BUF_SIZE - 1)

extern volatile u08 par_low_strobe_count;

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

extern void par_low_pulse_ack(u08 delay);

// BUSY (OUT)

inline void par_low_set_busy_lo(void)
{
  PAR_BUSY_PORT &= ~PAR_BUSY_MASK;
}

inline void par_low_set_busy_hi(void)
{
  PAR_BUSY_PORT |= PAR_BUSY_MASK;
}

inline void par_low_toggle_busy(void)
{
  if(PAR_BUSY_PORT & PAR_BUSY_MASK) {
    PAR_BUSY_PORT &= ~PAR_BUSY_MASK;    
  } else {
    PAR_BUSY_PORT |= PAR_BUSY_MASK;    
  }
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

// POUT (IN)

inline u08 par_low_get_pout(void)
{
  return (PAR_POUT_PIN & PAR_POUT_MASK) == PAR_POUT_MASK;
}

#endif