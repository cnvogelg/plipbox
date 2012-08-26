/*
 * spi.h - SPI setup
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

#ifndef SPI_H
#define SPI_H

#include <avr/io.h>
#include "global.h"

#ifdef HAVE_arduino
   
/* SPI config ATmega328

SPI_SS   = Digital 10 = PB2
SPI_MOSI = Digital 11 = PB3
SPI_MISO = Digital 12 = PB4
SPI_SCK  = Digital 13 = PB5

*/

#define SPI_SS_MASK		0x04
#define SPI_MOSI_MASK	0x08
#define SPI_MISO_MASK	0x10
#define SPI_SCK_MASK	0x20

#else

#ifdef HAVE_avrnetio
   
/* SPI config ATmega32

SPI_SS   = PB4
SPI_MOSI = PB5
SPI_MISO = PB6
SPI_SCK  = PB7

*/
   
#define SPI_SS_MASK   0x10
#define SPI_MOSI_MASK 0x20
#define SPI_MISO_MASK 0x40
#define SPI_SCK_MASK  0x80    
   
#endif
#endif

extern void spi_init(void);

extern void spi_out(u08 data);
extern u08  spi_in(void);

inline void spi_enable_eth(void) { PORTB &= ~SPI_SS_MASK; }
inline void spi_disable_eth(void) { PORTB |= SPI_SS_MASK; }

#endif
