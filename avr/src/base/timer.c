/*
 * timer.c - hw timer
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

#include "global.h"

#include <avr/interrupt.h>

// timer counter
volatile u16 timer_100us = 0;
volatile u16 timer_10ms = 0;
volatile u32 time_stamp = 0;
static u16 count;

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
  // prescale 64 
  // 16 MHz -> 250 KHz = 4 us timer
  
  // set to CTC on OCR1A with prescale 8
  TCCR1A = 0x00;
  TCCR1B = _BV(CS10) | _BV(CS11); // prescale 64
#ifdef TCCR1C
  TCCR1C = 0x00;
#endif
  
  // reset timer
  TCNT1 = 0;

  timer_100us = 0;
  timer_10ms = 0;
  time_stamp = 0;
  count = 0;

  sei();
}

// timer2 compare A handler
#ifdef TIMER2_COMPA_vect
ISR(TIMER2_COMPA_vect)
#else
ISR(TIMER2_COMP_vect)
#endif
{
  timer_100us++;
  time_stamp++;
  count++;
  if(count == 1000) {
    count = 0;
    timer_10ms++;
  }
} 

void timer_delay_10ms(u16 timeout)
{ timer_10ms=0; while(timer_10ms<timeout); }

void timer_delay_100us(u16 timeout)
{ timer_100us=0; while(timer_100us<timeout); }

// hw timer

u16 timer_hw_calc_rate_kbs(u16 bytes, u16 delta)
{
  if(delta != 0) {
    u32 nom = 1000 * (u32)bytes * 100;
    u32 denom = (u32)delta * 4; 
    u32 rate = nom / denom;
    return (u16)rate;
  } else {
    return 0;
  }
}


