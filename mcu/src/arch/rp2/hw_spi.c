/*
 * spi.c - SPI setup
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

#include "hardware/spi.h"

#include "hw_spi.h"

void hw_spi_init(void)
{
  // setup pin mux
  hw_spi_pins_init();

  // hi speed by default
  spi_init(spi0, 4000 * 1000);

  gpio_set_function(HW_SPI_MOSI_PIN, GPIO_FUNC_SPI);
  gpio_set_function(HW_SPI_MISO_PIN, GPIO_FUNC_SPI);
  gpio_set_function(HW_SPI_SCK_PIN, GPIO_FUNC_SPI);
}

void hw_spi_set_speed(u08 s)
{
  uint32_t speed;
  if(s == HW_SPI_SPEED_MAX) {
    speed = 4000 * 1000;
  } else {
    speed = 100 * 1000;
  }
  spi_set_baudrate(spi0, speed);
}

u08 hw_spi_xfer(u08 data)
{
  u08 ret;
  spi_write_read_blocking(spi0, &data, &ret, 1);
  return ret;
}
