BIN_DIR=../../bin
OBJ_DIR=../../obj

# build varian
CPUSUFFIX   = 000
BUILD_TYPE  = DEBUG
DEBUG_LEVEL = 4   # bitmask 1..8 max is 15
COMPILER    ?= vbcc
COMMON_DEFINES = -DENABLE_TIMING

# output directory
BUILD_DIR = $(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
OBJ_PATH = $(OBJ_DIR)/$(BUILD_DIR)
BIN_PATH = $(BIN_DIR)/$(BUILD_DIR)

# where do the amiga files reside?
# expects the following dirs:
# wb             - HD install of workbench 3.1
# sc             - install directory of SAS C 6.58 compiler
# roadshow       - Roadshow SDK installation
NDK_DIR?=$(HOME)/proj/amiga/NDK_3.2
export NDK_DIR

NDK_INC = $(NDK_DIR)/include_h
NDK_LIB = $(NDK_DIR)/lib
NDK_INC_ASM = $(NDK_DIR)/include_i

NDK_NET_DIR = $(NDK_DIR)/SANA+RoadshowTCP-IP
NDK_NET_INC = $(NDK_NET_DIR)/netinclude
NDK_DEV_INC = $(NDK_NET_DIR)/include

# include compiler setup
ifeq "$(COMPILER)" "vbcc"
include $(SCRIPT_DIR)/vbcc.mk
else
ifeq "$(COMPILER)" "sc"
include $(SCRIPT_DIR)/sc.mk
else
ifeq "$(COMPILER)" "gcc"
include $(SCRIPT_DIR)/gcc.mk
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
TEST_DIR ?= $(HOME)/extproj/plipbox

# macros

# --- def_tool ---
# $1 = var
# $2 = tool name
define def_tool
$1_PATH = $(BIN_PATH)/$2
$1_SRC = $2.c $3
$1_OBJ = $(patsubst %.c,$(OBJ_PATH)/%.o,$2.c $3)

TOOLS += $2
TOOL_PATHS += $(BIN_PATH)/$2
endef

# $1 = var
# $2 = tool name
define add_tool
$2: $($1_PATH)

clean-$2:
	@echo "  CLEAN $2"
	$(H)rm -f $($1_PATH) $($1_OBJ)

test-$2:
	@echo "  TEST $2"
	$(H)cp $($1_PATH) $(TEST_DIR)/

$($1_PATH): $($1_OBJ) $(LIBTOOL_PATH)
	@echo "  LNK $$@"
	$(H)$(LD) $(LDFLAGS_PRE) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$($1_OBJ)) \
	$(LDFLAGS_APP) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$$@)
endef
