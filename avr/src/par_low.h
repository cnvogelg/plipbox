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

#ifdef HAVE_arduino

/*
    Parallel Port Connection
                     Arduino        Nano
    DATA 0 ... 5     PC 0 ... 5     same      IN/OUT
    DATA 6 ... 7     PD 6 ... 7     same      IN/OUT

    /STROBE          PD 2           PD3       IN (INT0 / INT1)
    SELECT           PD 3           PB1       IN
    BUSY             PD 4           same      OUT
    POUT             PD 5           same      IN
    /ACK             PB 0           same      OUT
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

#ifdef HAVE_nano
// /STROBE (IN) (INT1) (D3)
#define PAR_STROBE_BIT          3
#define PAR_STROBE_MASK         _BV(PAR_STROBE_BIT)
#define PAR_STROBE_PORT         PORTD
#define PAR_STROBE_PIN          PIND
#define PAR_STROBE_DDR          DDRD
#else
// /STROBE (IN) (INT0) (D2)
#define PAR_STROBE_BIT          2
#define PAR_STROBE_MASK         _BV(PAR_STROBE_BIT)
#define PAR_STROBE_PORT         PORTD
#define PAR_STROBE_PIN          PIND
#define PAR_STROBE_DDR          DDRD
#endif

#ifdef HAVE_nano
// SELECT (IN) (D9)
#define PAR_SELECT_BIT          1
#define PAR_SELECT_MASK         _BV(PAR_SELECT_BIT)
#define PAR_SELECT_PORT         PORTB
#define PAR_SELECT_PIN          PINB
#define PAR_SELECT_DDR          DDRB
#else
// SELECT (IN) (D3)
#define PAR_SELECT_BIT          3
#define PAR_SELECT_MASK         _BV(PAR_SELECT_BIT)
#define PAR_SELECT_PORT         PORTD
#define PAR_SELECT_PIN          PIND
#define PAR_SELECT_DDR          DDRD
#endif

// BUSY (OUT) (D4)
#define PAR_BUSY_BIT            4
#define PAR_BUSY_MASK           _BV(PAR_BUSY_BIT)
#define PAR_BUSY_PORT           PORTD
#define PAR_BUSY_PIN            PIND
#define PAR_BUSY_DDR            DDRD

// POUT (IN) (D5)
#define PAR_POUT_BIT            5
#define PAR_POUT_MASK           _BV(PAR_POUT_BIT)
#define PAR_POUT_PORT           PORTD
#define PAR_POUT_PIN            PIND
#define PAR_POUT_DDR            DDRD

// /ACK (OUT) (D8)
#define PAR_ACK_BIT             0
#define PAR_ACK_MASK            _BV(PAR_ACK_BIT)
#define PAR_ACK_PORT            PORTB
#define PAR_ACK_PIN             PINB
#define PAR_ACK_DDR             DDRB

#else
#ifdef HAVE_avrnetio

/*
    Parallel Port Connection (AVR Net IO board)
                      AVR
    DATA 0 ... 7     PC 0 ... 7     IN/OUT

    /STROBE          PD 2           IN (INT0)
    SELECT           PA 3           IN
    POUT             PA 2           IN
    BUSY             PA 1           OUT
    /ACK             PA 0           OUT
*/

// lower bits of data
#define PAR_DATA_PORT           PORTC
#define PAR_DATA_PIN            PINC
#define PAR_DATA_DDR            DDRC

// /STROBE (IN) (INT0)
#define PAR_STROBE_BIT          2
#define PAR_STROBE_MASK         _BV(PAR_STROBE_BIT)
#define PAR_STROBE_PORT         PORTD
#define PAR_STROBE_PIN          PIND
#define PAR_STROBE_DDR          DDRD

// SELECT (IN) (INT1)
#define PAR_SELECT_BIT          3
#define PAR_SELECT_MASK         _BV(PAR_SELECT_BIT)
#define PAR_SELECT_PORT         PORTA
#define PAR_SELECT_PIN          PINA
#define PAR_SELECT_DDR          DDRA

// POUT (IN)
#define PAR_POUT_BIT            2
#define PAR_POUT_MASK           _BV(PAR_POUT_BIT)
#define PAR_POUT_PORT           PORTA
#define PAR_POUT_PIN            PINA
#define PAR_POUT_DDR            DDRA

// BUSY (OUT)
#define PAR_BUSY_BIT            1
#define PAR_BUSY_MASK           _BV(PAR_BUSY_BIT)
#define PAR_BUSY_PORT           PORTA
#define PAR_BUSY_PIN            PINA
#define PAR_BUSY_DDR            DDRA

// /ACK (OUT)
#define PAR_ACK_BIT             0
#define PAR_ACK_MASK            _BV(PAR_ACK_BIT)
#define PAR_ACK_PORT            PORTA
#define PAR_ACK_PIN             PINA
#define PAR_ACK_DDR             DDRA

#else
#ifdef HAVE_teensy20

/*
    Parallel Port Connection (Teensy 2.0)
                      AVR
    DATA 0 ... 7     PB 0 ... 7     IN/OUT

    /STROBE          PF 0           IN (INT0)
    SELECT           PF 1           IN
    POUT             PF 4           IN
    BUSY             PF 5           OUT
    /ACK             PF 6           OUT
*/

// lower bits of data
#define PAR_DATA_PORT           PORTB
#define PAR_DATA_PIN            PINB
#define PAR_DATA_DDR            DDRB

// /STROBE (IN) (INT0)
#define PAR_STROBE_BIT          0
#define PAR_STROBE_MASK         _BV(PAR_STROBE_BIT)
#define PAR_STROBE_PORT         PORTF
#define PAR_STROBE_PIN          PINF
#define PAR_STROBE_DDR          DDRF

// SELECT (IN) (INT1)
#define PAR_SELECT_BIT          6
#define PAR_SELECT_MASK         _BV(PAR_SELECT_BIT)
#define PAR_SELECT_PORT         PORTF
#define PAR_SELECT_PIN          PINF
#define PAR_SELECT_DDR          DDRF

// POUT (IN)
#define PAR_POUT_BIT            5
#define PAR_POUT_MASK           _BV(PAR_POUT_BIT)
#define PAR_POUT_PORT           PORTF
#define PAR_POUT_PIN            PINF
#define PAR_POUT_DDR            DDRF

// BUSY (OUT)
#define PAR_BUSY_BIT            4
#define PAR_BUSY_MASK           _BV(PAR_BUSY_BIT)
#define PAR_BUSY_PORT           PORTF
#define PAR_BUSY_PIN            PINF
#define PAR_BUSY_DDR            DDRF

// /ACK (OUT)
#define PAR_ACK_BIT             1
#define PAR_ACK_MASK            _BV(PAR_ACK_BIT)
#define PAR_ACK_PORT            PORTF
#define PAR_ACK_PIN             PINF
#define PAR_ACK_DDR             DDRF

#else
#error "Unknwon Board"
#endif
#endif
#endif

// ----- Input Buffer Handling -----

#define PAR_IN_BUF_BITS     4
#define PAR_IN_BUF_SIZE     (1 << PAR_IN_BUF_BITS)
#define PAR_IN_BUF_MASK     (PAR_IN_BUF_SIZE - 1)

// ----- Functions -----

extern void par_low_init(void);

// ----- Data Bus -----

extern void par_low_data_set_output(void);
extern void par_low_data_set_input(void);

#ifdef HAVE_arduino
inline void par_low_data_out(u08 d)
{
  PAR_DATA_LO_PORT = (d & PAR_DATA_LO_MASK) | (PAR_DATA_LO_PIN & ~PAR_DATA_LO_MASK);
  PAR_DATA_HI_PORT = (d & PAR_DATA_HI_MASK) | (PAR_DATA_HI_PIN & ~PAR_DATA_HI_MASK);
}

inline u08 par_low_data_in(void)
{
  u08 d1 = PAR_DATA_LO_PIN & PAR_DATA_LO_MASK;
  u08 d2 = PAR_DATA_HI_PIN & PAR_DATA_HI_MASK;
  return d1 | d2;
}
#else
#if defined(HAVE_avrnetio) || defined(HAVE_teensy20)
inline void par_low_data_out(u08 d)
{
  PAR_DATA_PORT = d;
}

inline u08 par_low_data_in(void)
{
  return PAR_DATA_PIN;
}
#endif
#endif

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
