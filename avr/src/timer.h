/*
 * timer.h - hw timer
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

#ifndef TIMER_H
#define TIMER_H

#include "global.h"

// init timers
void timer_init(void);

// a 100 Hz = 10ms timer
// 16bit: 0,10ms...~10hours
extern volatile u16 timer_10ms;
extern volatile u16 timer2_10ms;

// a 99.83ms ~ 100ms timer
// 16bit: 0,100us...6.5s
extern volatile u16 timer_100us;

// the master clock in seconds
extern volatile u32 clock_1s;

// busy wait with 10ms timer
extern void timer_delay_10ms(u16 timeout);

// busy wait with 100us timer
extern void timer_delay_100us(u16 timeout);
  
#endif

