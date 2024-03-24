
# output format
FORMAT = ihex

# ----- setup toolchain -----
# Define programs and commands.
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm
AVRDUDE = avrdude
RANLIB = avr-ranlib
AR = avr-ar

# compiler
CFLAGS = -g -std=gnu99 -fno-common
CFLAGS += -Os
#CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
#CFLAGS += -fno-inline
CFLAGS += -Wall -Werror -Wstrict-prototypes
CFLAGS += -mmcu=$(MCU)
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wa,-adhlns=$(OBJDIR)/$(notdir $(<:%.c=%.lst))
CFLAGS += -Wp,-M,-MP,-MT,$(OBJDIR)/$(*F).o,-MF,$(DEPDIR)/$(@F:.o=.d)
CFLAGS += $(FLAGS_COMMON)

# assembler
ASFLAGS = -g -mmcu=$(MCU)
#ASFLAGS = $(CFLAGS)
ASFLAGS += -Wa,-gstabs -x assembler-with-cpp
ASFLAGS += $(FLAGS_COMMON)

# linker
LDFLAGS += -g -mmcu=$(MCU)
LDFLAGS += -Wl,-Map=$(OUTPUT).map,--cref
LDFLAGS += -Wl,--gc-sections
LDLIBS += -lm -lc


# ----- setup flasher -----
# 'arduino' = Arduino bootloader via serial
ifeq "$(FLASHER)" "arduino"

LDR_SPEED ?= $(UART_BAUD)
LDR_SPEC = -P $(LDR_PORT) -b $(LDR_SPEED) -c arduino

else
# 'isp' = AVRISP mkII compatible, e.g. Diamex All AVR
ifeq "$(FLASHER)" "isp"

ISP_PORT ?= usb
ISP_TYPE ?= avrisp2
ISP_SPEED ?= 1
LDR_SPEC = -P $(ISP_PORT) -B $(ISP_SPEED) -c $(ISP_TYPE)

else
# 'usbasp' = see http://www.fischl.de/usbasp/
ifeq "$(FLASHER)" "usbasp"

LDR_SPEC = -c usbasp

else

$(error "Unsupported flasher '$(FLASHER)'!")

endif
endif
endif

ifeq "$(OS)" "Darwin"
LDR_PORT ?= $(shell ls /dev/cu.usbserial-* | tail -n 1)
else
LDR_PORT ?= $(shell ls /dev/ttyUSB* | tail -n 1)
endif

# commands
AVRDUDE_WRITE_FLASH  = -U flash:w:$(OUTPUT).hex
AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(OUTPUT).eep
AVRDUDE_WRITE_FUSE   = -U lfuse:w:$(LFUSE_$(MCU)):m -U hfuse:w:$(HFUSE_$(MCU)):m

AVRDUDE_LDR_FLAGS = -p $(FLASH_MCU) $(LDR_SPEC)

ifdef AVRDUDE_DEBUG
AVRDUDE_LDR_FLAGS += -v -v -v -v
endif
