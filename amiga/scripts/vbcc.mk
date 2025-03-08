# use vbcc and vasm
CC=vc +aos68km -c
LD=vc +aos68km
AS=vasmm68k_mot

VBCC_TARGET_AMIGAOS ?= $(VBCC)/targets/m68k-amigaos/
VBCC_INC = $(VBCC_TARGET_AMIGAOS)/include
VBCC_LIB = $(VBCC_TARGET_AMIGAOS)/lib

CFLAGS = -c99 -cpu=68$(CPUSUFFIX) -Os -+ -sc
CFLAGS += -I$(VBCC_INC) -I$(NDK_INC) -I$(NDK_NET_INC) -I$(NDK_DEV_INC)
CFLAGS += -I../../../shared/include -I../include
CFLAGS += $(COMMON_DEFINES)

OBJ_NAME = -o
CFLAGS_DEBUG = $(CFLAGS) -g -DDEBUG=$(DEBUG_LEVEL)
CFLAGS_RELEASE = $(CFLAGS)

LDFLAGS = -cpu=68$(CPUSUFFIX) -sc -L$(VBCC_LIB) -L$(NDK_LIB)
LDFLAGS += -lvc
LIBS_debug = -ldebug
LIBS = -lamiga
LDFLAGS_DEBUG = $(LDFLAGS) $(LIBS_debug) -g $(LIBS) -o
LDFLAGS_RELEASE = $(LDFLAGS) $(LIBS) -o
LDFLAGS_DEV = -nostdlib
LDFLAGS_APP =
LDFLAGS_HAS_MAP = 0

ASFLAGS = -Fhunk -quiet -phxass -m68$(CPUSUFFIX) -I$(NDK_INC_ASM)

AR = cat
AR_OUT = >
LIB_EXT = .lib
