#ifndef HW_SPI_COMMON_H
#define HW_SPI_COMMON_H

#define HW_SPI_OK           0
#define HW_SPI_IN_USE       1

#define HW_SPI_SPEED_MAX    0
#define HW_SPI_SPEED_SLOW   1

INLINE u08 hw_spi_get_num_cs(void)
{
  return CONFIG_SPI_NUM_CS;
}

extern void hw_spi_enable_cs(u08 cs_id);
extern void hw_spi_disable_cs(u08 cs_id);

extern u08  hw_spi_acquire(u08 cs_id);
extern void hw_spi_release(u08 cs_id);

#endif
