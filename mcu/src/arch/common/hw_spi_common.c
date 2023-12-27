#include "autoconf.h"
#include "types.h"

#include "hw_spi.h"

static u08 spi_use = 0;

void hw_spi_enable_cs(u08 cs_id)
{
  if(cs_id == 0) {
    hw_spi_enable_cs0();
  } else {
    hw_spi_enable_cs1();
  }
}

void hw_spi_disable_cs(u08 cs_id)
{
  if(cs_id == 0) {
    hw_spi_disable_cs0();
  } else {
    hw_spi_disable_cs1();
  }
}

u08 hw_spi_acquire(u08 cs_id)
{
  u08 mask = 1 << cs_id;
  // already in use?
  if((spi_use & mask) == mask) {
    return HW_SPI_IN_USE;
  }
  // need to init spi?
  if(spi_use == 0) {
    hw_spi_init();
  }
  spi_use |= mask;
  return HW_SPI_OK;
}

void hw_spi_release(u08 cs_id)
{
  u08 mask = 1 << cs_id;
  spi_use &= ~mask;
}
