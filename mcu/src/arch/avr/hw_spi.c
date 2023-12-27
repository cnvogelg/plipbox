/*
 * hw_spi.c - SPI setup
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

#include "hw_spi.h"

void hw_spi_init(void)
{
  // output: SS, MOSI, SCK
  DDRB |= SPI_SS_MASK | SPI_MOSI_MASK | SPI_SCK_MASK;
  // input: MISO
  DDRB &= ~(SPI_MISO_MASK);

  // MOSI, SCK = 0
  PORTB &= ~(SPI_MOSI_MASK | SPI_SCK_MASK);
  // SS = 1
  PORTB |= SPI_SS_MASK;

  // setup SS1
  SPI_SS1_DDR  |= SPI_SS1_MASK;
  SPI_SS1_PORT |= SPI_SS1_MASK;

  SPCR = _BV(SPE) | _BV(MSTR); // 8 MHz @ 16
  SPSR = _BV(SPI2X);
}

void hw_spi_set_speed(u08 speed)
{
  if(speed == HW_SPI_SPEED_MAX) {
    SPCR = _BV(SPE) | _BV(MSTR); // 8 MHz @ 16 MHz FPU (clk/2)
    SPSR = _BV(SPI2X);
  } else {
    // (clk/128)  @16 MHz -> 125 KHz
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1) | _BV(SPR0);
    SPSR = 0;
  }
}

