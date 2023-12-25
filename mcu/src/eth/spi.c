/*
 * spi.c - SPI setup
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

#include "spi.h"

void spi_init(void)
{
	// output: SS, MOSI, SCK
	DDRB |= SPI_SS_MASK | SPI_MOSI_MASK | SPI_SCK_MASK;
	// input: MISO
	DDRB &= ~(SPI_MISO_MASK);
	
	// MOSI, SCK = 0
	PORTB &= ~(SPI_MOSI_MASK | SPI_SCK_MASK);
	// SS = 1
	PORTB |= SPI_SS_MASK;

  SPCR = _BV(SPE) | _BV(MSTR); // 8 MHz @ 16
	SPSR = _BV(SPI2X); 
}
