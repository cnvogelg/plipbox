/*
 * timer.c - hw timer
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of parbox.
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

#include <avr/interrupt.h>

#include "types.h"
#include "hw_timer.h"

volatile hw_timer_ms_t  timer_ms;

void hw_timer_init(void)
{
  cli();

  // ----- TIMER0 (8bit) -----

  // clear timer/counter on compare0 match
#ifdef TCCR0A
  TCCR0A = _BV(WGM01); // CTC
  TCCR0B = _BV(CS02); // prescale 256
#else
  TCCR0 = _BV(WGM01) | _BV(CS02); // CTC + prescale 256
#endif

  // prescale = 256
  // t_tick = prescale / F_CPPU
  //
  // how many ticks n until 1ms is reached?
  //
  // t_tick * n = 1 ms = 1s / 1000
  //
  // -> n = F_CPU / ( prescaler * 1000 )
  // -> compare val m = n -1
  //
  // e.g. F_CPU = 16000000 -> m = 61
  //      F_CPU = 20000000 -> m = 77

#define TIMER0_PRESCALE 256
#define TIMER0_COMPARE_VAL (F_CPU / (TIMER0_PRESCALE * 1000UL)) - 1
#ifdef OCR0A
  OCR0A = TIMER0_COMPARE_VAL;
#else
  OCR0  = TIMER0_COMPARE_VAL;
#endif

  // reset timer
  TCNT0  = 0x00;

  // enable Output Compare 0 overflow interrupt
#ifdef TIMSK0
  TIMSK0 = _BV(OCIE0A);
#else
  TIMSK |= _BV(OCIE0);
#endif

  sei();
}

// timer1 compare A handler
#ifdef TIMER0_COMPA_vect
ISR(TIMER0_COMPA_vect)
#else
ISR(TIMER0_COMP_vect)
#endif
{
  timer_ms++;
}
