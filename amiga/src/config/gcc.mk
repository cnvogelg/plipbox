# use gcc and vasm
CC=m68k-amigaos-gcc -c
LD=m68k-amigaos-gcc
AS=vasmm68k_mot

# NDK includes/libs
NDK_DIR ?= $(AMIGA_DIR)/ndk_3.9
NDK_INC = $(NDK_DIR)/include/include_h
NDK_LIB = $(NDK_DIR)/include/linker_libs
NDK_INC_ASM = $(NDK_DIR)/include/include_i

# netinclude
NET_INC ?= $(AMIGA_DIR)/roadshow/netinclude
DEV_INC ?= $(AMIGA_DIR)/roadshow/include

CFLAGS = -Wall -Werror -noixemul -mcrt=clib2
CFLAGS += -mcpu=68$(CPUSUFFIX) -fbaserel -Os
CFLAGS += -I$(VBCC_INC) -I$(NDK_INC) -I$(NET_INC) -I$(DEV_INC)
CFLAGS += -I$(DEVICE_NAME) -I.

CFLAGS += -DDEVICE_NAME="\"\\\"$(DEVICE_NAME).device\\\"\""
CFLAGS += -DDEVICE_VERSION=$(DEVICE_VERSION)
CFLAGS += -DDEVICE_REVISION=$(DEVICE_REVISION)
CFLAGS += -DDEVICE_ID="\"\\\"$(DEVICE_ID)\\\"\""

OBJ_NAME = -o
CFLAGS_DEBUG = $(CFLAGS) -g
CFLAGS_RELEASE = $(CFLAGS)

LDFLAGS = -mcpu=68$(CPUSUFFIX) -fbaserel -L$(VBCC_LIB) -L$(NDK_LIB)
LDFLAGS += -Os
LIBS_debug = -ldebug
LIBS = -lamiga -lc
LDFLAGS_DEBUG = $(LDFLAGS) $(LIBS_debug) -g $(LIBS) -o
LDFLAGS_RELEASE = $(LDFLAGS) $(LIBS) -o
LDFLAGS_DEV = -nostartfiles
LDFLAGS_APP =
LDFLAGS_HAS_MAP = 0

ASFLAGS = -Fhunk -quiet -phxass -m68$(CPUSUFFIX) -I$(NDK_INC_ASM)
