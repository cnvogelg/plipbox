/*
 * arduino2009.h - arduino2009 hardware access
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

#ifndef ARDU_NANO_BOARD_H
#define ARDU_NANO_BOARD_H

#include "global.h"
#include <avr/io.h>

// ----- BOARD -----
void board_init(void);

// ----- LEDs -----
#define LED_MASK        _BV(5)

// set led on
#define led_off()       { PORTB &= ~LED_MASK; }
// set led off 
#define led_on()        { PORTB |= LED_MASK; }

// ----- RTS/CTS -----
#define uart_init_rts_cts() // not required
#define uart_set_cts(x)     do {} while(0)
#define uart_get_rts()      1

#endif

