BIN_DIR=../../bin
OBJ_DIR=../../obj

# build varian
CPUSUFFIX   = 000
BUILD_TYPE  = DEBUG
DEBUG_LEVEL = 9   # bitmask 1..8 max is 15
COMPILER    ?= vbcc

# output directory
BUILD_DIR = build_$(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
BUILD_PATH = $(OBJ_DIR)/$(BUILD_DIR)

# where do the amiga files reside?
# expects the following dirs:
# wb             - HD install of workbench 3.1
# sc             - install directory of SAS C 6.58 compiler
# roadshow       - Roadshow SDK installation
AMIGA_DIR?=$(HOME)/projects/amidev
NETINC=$(AMIGA_DIR)/roadshow
export AMIGA_DIR

# include compiler setup
ifeq "$(COMPILER)" "vbcc"
include ../config/vbcc.mk
else
ifeq "$(COMPILER)" "sc"
include ../config/sc.mk
else
ifeq "$(COMPILER)" "gcc"
include ../config/gcc.mk
else
$(error Invalid compiler $(COMPILER))
endif
endif
endif

# show compile
ifeq "$(V)" "1"
H :=
else
H := @
endif

# where to put test files
TEST_DIR ?= $(HOME)/proj/plipbox
