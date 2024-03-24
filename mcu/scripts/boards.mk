# define the target boards

ALL_BOARDS=arduino avrnetio nano picow
DIST_BOARDS=$(ALL_BOARDS)

# select default board
BOARD ?= nano

# ----- avr boards -----
ifeq "$(BOARD)" "arduino"

MCU = atmega328
ARCH = avr
MACH = ardunano
FLASH_MCU = m328p
F_CPU = 16000000
#MAX_SIZE = 14336
MAX_SIZE = 30720
MAX_SRAM = 2048
#UART_BAUD = 500000
#UART_BAUD = 19200
#UART_BAUD = 250000
UART_BAUD = 57600
FLASHER = arduino

else
ifeq "$(BOARD)" "nano"

MCU = atmega328
ARCH = avr
MACH = ardunano
FLASH_MCU = m328p
F_CPU = 16000000
MAX_SIZE = 30720
MAX_SRAM = 2048
UART_BAUD = 57600
FLASHER = isp

else
ifeq "$(BOARD)" "avrnetio"

MCU = atmega32
ARCH = avr
MACH = avrnetio
FLASH_MCU = m32
F_CPU = 16000000
MAX_SIZE = 32768
MAX_SRAM = 2048
UART_BAUD = 57600
FLASHER = isp

else

# ----- rp2 boards -----
ifeq "$(BOARD)" "picow"

MCU = cortexm0
ARCH = rp2
MACH = pico

else

$(error "Unsupported board '$(BOARD)'. Only '$(ALL_BOARDS)'' allowed!")

endif
endif
endif
endif
