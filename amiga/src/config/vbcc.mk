# use vbcc and vasm
CC=vc +aos68km -c
LD=vc +aos68km
AS=vasmm68k_mot

# NDK includes/libs
NDK_DIR ?= $(AMIGA_DIR)/ndk_3.9
NDK_INC = $(NDK_DIR)/include/include_h
NDK_LIB = $(NDK_DIR)/include/linker_libs
NDK_INC_ASM = $(NDK_DIR)/include/include_i

# netinclude
NET_INC ?= $(AMIGA_DIR)/roadshow/netinclude
DEV_INC ?= $(AMIGA_DIR)/roadshow/include

VBCC_TARGET_AMIGAOS ?= $(VBCC)/targets/m68k-amigaos/
VBCC_INC = $(VBCC_TARGET_AMIGAOS)/include
VBCC_LIB = $(VBCC_TARGET_AMIGAOS)/lib

CFLAGS = -c99 -cpu=68$(CPUSUFFIX) -Os -+ -sc
CFLAGS += -I$(VBCC_INC) -I$(NDK_INC) -I$(NET_INC) -I$(DEV_INC)
CFLAGS += -I$(DEVICE_NAME) -I.

CFLAGS += -DDEVICE_NAME="\"\\\"$(DEVICE_NAME).device\\\"\""
CFLAGS += -DDEVICE_VERSION=$(DEVICE_VERSION)
CFLAGS += -DDEVICE_REVISION=$(DEVICE_REVISION)
CFLAGS += -DDEVICE_ID="\"\\\"$(DEVICE_ID)\\\"\""

OBJ_NAME = -o
CFLAGS_DEBUG = $(CFLAGS) -g -DDEBUG=$(DEBUG_LEVEL)
CFLAGS_RELEASE = $(CFLAGS)

LDFLAGS = -cpu=68$(CPUSUFFIX) -sc -L$(VBCC_LIB) -L$(NDK_LIB)
LDFLAGS += -lvc -Os
LIBS_debug = -ldebug
LIBS = -lamiga
LDFLAGS_DEBUG = $(LDFLAGS) $(LIBS_debug) -g $(LIBS) -o
LDFLAGS_RELEASE = $(LDFLAGS) $(LIBS) -o
LDFLAGS_DEV = -nostdlib
LDFLAGS_APP =
LDFLAGS_HAS_MAP = 0

ASFLAGS = -Fhunk -quiet -phxass -m68$(CPUSUFFIX) -I$(NDK_INC_ASM)
