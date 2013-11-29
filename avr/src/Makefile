#
# Makefile - plipbox mcu makefile
#
# Written by
#  Christian Vogelgsang <chris@vogelgsang.org>
#
# This file is part of plipbox.
# See README for copyright notice.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
#  02111-1307  USA.
#

# ----- Config Options -----
 
OS=$(shell uname -s)

# install dir of avr libc (here MacPorts location)
ifeq "$(OS)" "Darwin"
AVRLIBC_DIR = /opt/local/avr
else
AVRLIBC_DIR = /usr/lib/avr
endif

ALL_BOARDS= arduino avrnetio nano
DIST_BOARDS= arduino avrnetio nano

# select board
BOARD ?= nano
DEBUG ?= 0

ifeq "$(BOARD)" "arduino"

MCU = atmega328
FLASH_MCU = m328p
F_CPU = 16000000
#MAX_SIZE = 14336
MAX_SIZE = 30720
MAX_SRAM = 1024
#UART_BAUD = 500000
#UART_BAUD = 19200
#UART_BAUD = 250000
UART_BAUD = 57600

LDR_PROG = arduino
#LDR_SPEED = 19200
LDR_SPEED = 57600
LDR_SPEC = -P $(LDR_PORT) -b $(LDR_SPEED) -c $(LDR_PROG)

else
ifeq "$(BOARD)" "nano"

MCU = atmega328
FLASH_MCU = m328p
F_CPU = 16000000
MAX_SIZE = 30720
MAX_SRAM = 1024
UART_BAUD = 57600

LDR_PROG = arduino
LDR_SPEED = 57600
LDR_SPEC = -P $(LDR_PORT) -b $(LDR_SPEED) -c $(LDR_PROG)

# its a subclass of the Arduino board
DEFINES += HAVE_arduino
BOARDFILE = arduino.c

else
ifeq "$(BOARD)" "avrnetio"

MCU = atmega32
FLASH_MCU = m32
F_CPU = 16000000
MAX_SIZE = 32768
MAX_SRAM = 1024
UART_BAUD = 57600
LDR_PROG = usbasp
LDR_SPEC = -c $(LDR_PROG)

else

$(error "Unsupported board '$(BOARD)'. Only $(ALL_BOARDS) allowed!")

endif
endif
endif

# ----- End of Config -----

# mainfile/project name
PROJECT = plipbox
VERSION_MAJ = 0
VERSION_MIN = 5
VERSION_EXTRA =
BUILD_DATE = $(shell date '+%Y%m%d')

# combine version
VERSION=$(VERSION_MAJ).$(VERSION_MIN)$(VERSION_EXTRA)

# basename of output
BASENAME = $(PROJECT)-$(VERSION)-$(UART_BAUD)-$(BOARD)-$(MCU)
# project
OUTPUT = $(OUTDIR)/$(BASENAME)
# dist dir
DISTDIR = ../bin

# source files
BOARDFILE ?= $(BOARD).c
SRC := board/$(BOARDFILE)
SRC += util.c uart.c uartutil.c timer.c
SRC += par_low.c pb_proto.c
SRC += pkt_buf.c param.c
SRC += spi.c enc28j60.c 
SRC += net/net.c net/arp.c
SRC += eth_io.c eth_state.c dump.c
SRC += pb_io.c pb_state.c
SRC += cmd.c cmd_table.c cmdkey_table.c
SRC += main.c stats.c log.c

# output format
FORMAT = ihex

# build directory
BUILD = BUILD
OBJDIR = $(BUILD)/$(BASENAME)/obj
DEPDIR = $(OBJDIR)
OUTDIR = $(BUILD)

# target files
OBJ = $(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

# compiler switches
CFLAGS = -g -std=gnu99
CFLAGS += -Os
#CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
#CFLAGS += -fno-inline
CFLAGS += -Wall -Werror -Wstrict-prototypes
CFLAGS += -I$(AVRLIBC_DIR)/include
CFLAGS += -mmcu=$(MCU) -I. 

CFLAGS_LOCAL = -Wa,-adhlns=$(<:%.c=$(OBJDIR)/%.lst)
CFLAGS_LOCAL += -Wp,-M,-MP,-MT,$(OBJDIR)/$(*F).o,-MF,$(DEPDIR)/$(@F:.o=.d)
CFLAGS_LOCAL += -DVERSION_MIN="$(VERSION_MIN)" -DVERSION_MAJ="$(VERSION_MAJ)"
CFLAGS_LOCAL += -DVERSION="\"$(VERSION)\"" -DBUILD_DATE="\"$(BUILD_DATE)\""
CFLAGS_LOCAL += -DHAVE_$(BOARD) -DUART_BAUD=$(UART_BAUD) -DBOARD=$(BOARD)
CFLAGS_LOCAL += -DF_CPU=$(F_CPU)
CFLAGS_LOCAL += $(patsubst %,-D%,$(DEFINES))
ifeq "$(DEBUG)" "1"
CFLAGS_LOCAL += -DDEBUG
endif

# linker switches
LDFLAGS = -Wl,-Map=$(OUTPUT).map,--cref
LDFLAGS = -lm -lc

# Define programs and commands.
SHELL = sh
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm
AVRDUDE = avrdude
REMOVE = rm -f
COPY = cp
RANLIB = avr-ranlib
AR = avr-ar

# define V to see compile output
ifdef V
HIDE=
else
HIDE=@
endif

# ---------- RULES ----------

all: build

help:
	@echo "$(OS)"
	@echo
	@echo "  supported boards: $(ALL_BOARDS)"
	@echo
	@echo "build [BOARD=<board>]"
	@echo "prog [BOARD=<board>]"
	@echo "clean"

dirs:
	@if [ ! -d $(BUILD) ]; then mkdir -p $(BUILD); fi 
	@if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi
	@if [ ! -d $(OBJDIR)/net ]; then mkdir -p $(OBJDIR)/net; fi
	@if [ ! -d $(OBJDIR)/board ]; then mkdir -p $(OBJDIR)/board; fi
	@if [ ! -d $(DEPDIR) ]; then mkdir -p $(DEPDIR); fi
	@if [ ! -d $(OUTDIR) ]; then mkdir -p $(OUTDIR); fi

build: dirs hdr hex lss size

hdr:
	@echo "--- building BOARD=$(BOARD) F_CPU=$(F_CPU) MCU=$(MCU) FLASH_MCU=$(FLASH_MCU) ---"
	@echo "             UART_BAUD=$(UART_BAUD) DEFINES=$(DEFINES)"

elf: $(OUTPUT).elf $(OUTPUT).lss $(OUTPUT).sym
	@echo "  --- resulting elf ---"
	$(HIDE)$(SIZE) $(OUTPUT).elf

hex: $(OUTPUT).hex

lss: $(OUTPUT).lss

size: size_code size_data

size_code: $(OUTPUT).hex
	@SIZE=`$(SIZE) -A --target=$(FORMAT) $(OUTPUT).hex | grep Total | awk '{ print $$2 }'` ; \
	if [ $$SIZE -gt $(MAX_SIZE) ] ; then \
		echo "  $$SIZE >  $(MAX_SIZE) bytes: code TOO LARGE" ; exit 1 ; \
	else \
		echo "  $$SIZE <= $(MAX_SIZE) bytes: code ok" ; \
	fi

size_data: $(OUTPUT).elf
	@DATA=`$(SIZE) -A $(OUTPUT).elf | fgrep .data | awk '{ print $$2 }'` ; \
	BSS=`$(SIZE) -A $(OUTPUT).elf | fgrep .bss | awk '{ print $$2 }'` ; \
	SUM=$$(( $$DATA + $$BSS )) ; \
	if [ $$SUM -gt $(MAX_SRAM) ] ; then \
		echo "  $$SUM >  $(MAX_SRAM) bytes: sram TOO LARGE" ; exit 1 ; \
	else \
		echo "  $$SUM <= $(MAX_SRAM) bytes: sram ok" ; \
	fi

clean:
	rm -rf $(BUILD)

clean_dist: clean
	rm -rf $(DISTDIR)

build_all: clean
	@for a in $(DIST_BOARDS) ; do \
		$(MAKE) build BOARD=$$a ;\
	done
	$(MAKE) build BOARD=arduino MCU=atmega328

dist: build_all
	@rm -rf $(DISTDIR)
	@mkdir -p $(DISTDIR)
	@cp $(BUILD)/*.hex $(DISTDIR)
	@rm -rf $(BUILD)
	@ls -la $(DISTDIR)

# ----- Helper Rules -----

# final hex (flash) file from elf
%.hex: %.elf
	@echo "  making hex $@"
	$(HIDE)$(OBJCOPY) -O $(FORMAT) -j .data -j .text $< $@

# finale eeprom file from elf
%.eep: %.elf
	@echo "  making eep $@"
	$(HIDE)$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# extended listing file
%.lss: %.elf
	@echo "  generating listing $@"
	$(HIDE)$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo "  generating symbols $@"
	$(HIDE)$(NM) -n $< > $@

# link
%.elf: $(OBJ)
	@echo "  linking elf $@"
	$(HIDE)$(CC) $(CFLAGS) $(OBJ) --output $@ $(LDFLAGS)

# compile
$(OBJDIR)/%.o : %.c
	@echo "  compiling $<"
	$(HIDE)$(CC) -c $(CFLAGS) $(CFLAGS_LOCAL) $< -o $@ 

# include dependencies
-include $(shell mkdir -p $(DEPDIR) 2>/dev/null) $(wildcard $(DEPDIR)/*.d)

.PRECIOUS: $(OBJ)
.PHONY: all dirs elf hex prog clean avrlib clean.edit hdr size_code size_data size

# ----- AVRdude --------------------------------------------------------------

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

check_prog:
	$(AVRDUDE) $(AVRDUDE_LDR_FLAGS) -U signature:r:sig.txt:h
	@echo -n " device signature: "
	@cat sig.txt
	@rm -f sig.txt

prog: $(OUTPUT).hex size
	@echo "  --- programming flash ---"
	$(AVRDUDE) $(AVRDUDE_LDR_FLAGS) $(AVRDUDE_WRITE_FLASH)

read_fuse:
	$(AVRDUDE) $(AVRDUDE_LDR_FLAGS) -U lfuse:r:lfuse.txt:h -U hfuse:r:hfuse.txt:h
	@echo -n " lfuse: " 
	@cat lfuse.txt
	@echo -n " hfuse: "
	@cat hfuse.txt
	@rm -f lfuse.txt hfuse.txt
