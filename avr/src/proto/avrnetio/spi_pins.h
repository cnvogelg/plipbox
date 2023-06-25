/* SPI config ATmega32

SPI_SS   = PB4
SPI_MOSI = PB5
SPI_MISO = PB6
SPI_SCK  = PB7

SPI_SS1  = PD3 (?)

*/

#define SPI_SS_MASK   0x10
#define SPI_MOSI_MASK 0x20
#define SPI_MISO_MASK 0x40
#define SPI_SCK_MASK  0x80

#define SPI_SS1_DDR   DDRD
#define SPI_SS1_PORT  PORTD
#define SPI_SS1_MASK  0x04
