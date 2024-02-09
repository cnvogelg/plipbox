#ifndef HW_SPI_PINS_H
#define HW_SPI_PINS_H

#include "pico/stdlib.h"
#include "arch.h"

/* SPI Port Setup

              Pico Pins
   MOSI          19         spi0
   MISO          16         spi0
   SCK           18         spi0
   CS0           17
   CS1           22
*/

#define HW_SPI_CS0_PIN     17
#define HW_SPI_CS1_PIN     22
#define HW_SPI_MOSI_PIN    19
#define HW_SPI_MISO_PIN    16
#define HW_SPI_SCK_PIN     18

INLINE void hw_spi_pins_init(void)
{
  // cs0 out hi
  gpio_init(HW_SPI_CS0_PIN);
  gpio_put(HW_SPI_CS0_PIN, true);
  gpio_set_dir(HW_SPI_CS0_PIN, GPIO_OUT);

  // cs1 out hi
  gpio_init(HW_SPI_CS1_PIN);
  gpio_put(HW_SPI_CS1_PIN, true);
  gpio_set_dir(HW_SPI_CS1_PIN, GPIO_OUT);
}

FORCE_INLINE void hw_spi_pins_cs0_hi(void)
{
  gpio_put(HW_SPI_CS0_PIN, true);
}

FORCE_INLINE void hw_spi_pins_cs0_lo(void)
{
  gpio_put(HW_SPI_CS0_PIN, false);
}

FORCE_INLINE void hw_spi_pins_cs1_hi(void)
{
  gpio_put(HW_SPI_CS1_PIN, true);
}

FORCE_INLINE void hw_spi_pins_cs1_lo(void)
{
  gpio_put(HW_SPI_CS1_PIN, false);
}

#endif
