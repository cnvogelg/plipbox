/*
 * timer.c - hw timer
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plip2slip.
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

#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>

void timer_init(void)
{
  cli();
  
  // ----- TIMER2 (8bit) -----
  // 100us timer
  
  // clear timer/counter on compare0 match
#ifdef TCCR2A
  TCCR2A = _BV(WGM21); // CTC
  TCCR2B = _BV(CS21); // prescale 8
#else
  TCCR2 = _BV(WGM21) | _BV(CS21); // CTC + prescale 8
#endif

  // t_tick = prescale / F_CPPU = 8 / F_CPU
  // how many ticks n until 100 us are reached?
  //
  // t_tick * n = 100 us
  //
  // -> n = 100 * F_CPU / ( prescaler * 1000000 )
  // -> compare val m = n -1
  
#define TIMER2_COMPARE_VAL ((100 * F_CPU) / (8 * 1000000)) - 1
#ifdef OCR2A
  OCR2A = TIMER2_COMPARE_VAL;
#else
  OCR2  = TIMER2_COMPARE_VAL;
#endif

  // reset timer
  TCNT2  = 0x00;
  
  // enable Output Compare 0 overflow interrupt
#ifdef TIMSK2
  TIMSK2 = _BV(OCIE2A);
#else
  TIMSK |= _BV(OCIE2);
#endif

  // ----- TIMER1 (16bit) -----
  // 10ms counter
  
  // set to CTC on OCR1A with prescale 8
  TCCR1A = 0x00;
  TCCR1B = _BV(WGM12) | _BV(CS11); // CTC + prescale 8
#ifdef TCCR1C
  TCCR1C = 0x00;
#endif

  // t_tick = prescale / F_CPPU = 8 / F_CPU
  // how many ticks n until 100 us are reached?
  //
  // t_tick * n = 10 ms
  //
  // -> n = 10 * F_CPU / ( prescaler * 1000 )
  // -> compare val m = n -1

#define TIMER1_COMPARE_VAL  ((10 * F_CPU) / (8 * 1000)) - 1
#ifdef OCR1A
  OCR1A = TIMER1_COMPARE_VAL;
#else
  OCR1 = TIMER1_COMPARE_VAL;
#endif
  
  // reset timer
  TCNT1 = 0;

#ifdef TIMSK1
  // generate interrupt for OCIE1A
  TIMSK1 = _BV(OCIE1A);
#else
  TIMSK |= _BV(OCIE1A);
#endif

  sei();
}

// timer counter
volatile u16 timer_100us = 0;
volatile u16 timer_10ms = 0;
volatile u16 timer2_10ms = 0;
volatile u32 clock_1s = 0;
static u08 clock_10ms = 0;

// timer2 compare A handler
#ifdef TIMER2_COMPA_vect
ISR(TIMER2_COMPA_vect)
#else
ISR(TIMER2_COMP_vect)
#endif
{
  timer_100us++;
}

// timer1 compare A handler
ISR(TIMER1_COMPA_vect)
{
  timer_10ms++;
  timer2_10ms++;

  clock_10ms++;
  if(clock_10ms == 100) {
    clock_1s ++;
    clock_10ms = 0;
  }
}

void timer_delay_10ms(u16 timeout)
{ timer_10ms=0; while(timer_10ms<timeout); }

void timer_delay_100us(u16 timeout)
{ timer_100us=0; while(timer_100us<timeout); }
