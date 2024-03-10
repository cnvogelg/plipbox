# use gcc and vasm
CC=m68k-amigaos-gcc -c
LD=m68k-amigaos-gcc
AS=vasmm68k_mot

# NDK includes/libs
NDK_DIR ?= $(AMIGA_DIR)/ndk3.2r4
NDK_INC = $(NDK_DIR)/include_h
NDK_LIB = $(NDK_DIR)/lib
NDK_INC_ASM = $(NDK_DIR)/include_i

# netinclude
NET_INC ?= $(AMIGA_DIR)/roadshow/netinclude
DEV_INC ?= $(AMIGA_DIR)/roadshow/include

BASEREL = -fbaserel -DBASEREL

CFLAGS = -Wall -Werror -noixemul -mcrt=clib2
CFLAGS += -mcpu=68$(CPUSUFFIX) $(BASEREL) -Os
CFLAGS += -I$(VBCC_INC) -I$(NDK_INC) -I$(NET_INC) -I$(DEV_INC)
CFLAGS += -I$(DEVICE_NAME) -I$(DEVICE_NAME)/proto -I. -I../include

CFLAGS += -DDEVICE_NAME="\"\\\"$(DEVICE_NAME).device\\\"\""
CFLAGS += -DDEVICE_VERSION=$(DEVICE_VERSION)
CFLAGS += -DDEVICE_REVISION=$(DEVICE_REVISION)
CFLAGS += -DDEVICE_ID="\"\\\"$(DEVICE_ID)\\\"\""

OBJ_NAME = -o
CFLAGS_DEBUG = $(CFLAGS) -g
CFLAGS_RELEASE = $(CFLAGS)

LDFLAGS = -mcpu=68$(CPUSUFFIX) $(BASEREL) -L$(NDK_LIB)
LDFLAGS += -Os
LIBS_debug = -ldebug
LIBS = -lamiga -lc
LDFLAGS_DEBUG = $(LDFLAGS) $(LIBS_debug) -g $(LIBS) -o
LDFLAGS_RELEASE = $(LDFLAGS) $(LIBS) -o
LDFLAGS_DEV = -nostartfiles
LDFLAGS_APP =
LDFLAGS_HAS_MAP = 0

ASFLAGS = -Fhunk -quiet -phxass -m68$(CPUSUFFIX) -I$(NDK_INC_ASM)
