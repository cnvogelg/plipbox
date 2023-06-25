/* SPI config ATmega328

SPI_SS   = Digital 10 = PB2
SPI_MOSI = Digital 11 = PB3
SPI_MISO = Digital 12 = PB4
SPI_SCK  = Digital 13 = PB5

SPI_SS1  = PD2

*/

#define SPI_SS_MASK     0x04
#define SPI_MOSI_MASK   0x08
#define SPI_MISO_MASK   0x10
#define SPI_SCK_MASK    0x20

#define SPI_SS1_DDR     DDRD
#define SPI_SS1_PORT    PORTD
#define SPI_SS1_MASK    0x04
