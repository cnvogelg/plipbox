# gnu makefile to trigger build via vamos

BIN_DIR=../bin
OBJ_DIR=../obj

# build varian
CPUSUFFIX   = 000
BUILD_TYPE  = DEBUG
DEBUG_LEVEL = 1   # bitmask 1..8 max is 15
COMPILER    ?= vbcc

# where do the amiga files reside?
# expects the following dirs:
# wb             - HD install of workbench 3.1
# sc             - install directory of SAS C 6.58 compiler
# roadshow       - Roadshow SDK installation
AMIGA_DIR?=$(HOME)/projects/amidev
NETINC=$(AMIGA_DIR)/roadshow
export AMIGA_DIR

# device file
include ../../version.mk
DEVICE_NAME = plipbox
DEVICE_VERSION = $(VERSION_MAJ)
DEVICE_REVISION = $(VERSION_MIN)
DEVICE_DATE = $(shell date '+%d.%m.%Y')
DEVICE_FILE = $(DEVICE_NAME).device_$(BUILD_TYPE)_$(CPUSUFFIX)
DEVICE_ID = $(DEVICE_NAME) $(DEVICE_VERSION).$(DEVICE_REVISION) ($(DEVICE_DATE))

# include compiler setup
ifeq "$(COMPILER)" "vbcc"
include config/vbcc.mk
else
ifeq "$(COMPILER)" "sc"
include config/sc.mk
else
$(error Invalid compiler $(COMPILER))
endif
endif

# output directory
BUILD_DIR = build_$(BUILD_TYPE)_$(CPUSUFFIX)
BUILD_PATH = $(OBJ_DIR)/$(BUILD_DIR)

# generic source files
CSRC=rt.c device.c server.c track.c

# driver specific source files
CSRC_$(DRIVER_NAME) = hw.c
ASRC_$(DRIVER_NAME) = hwpar.asm

# object files
OBJS+=$(patsubst %.c,$(BUILD_PATH)/%.o,$(CSRC) $(CSRC_$(DRIVER_NAME)))
OBJS+=$(patsubst %.asm,$(BUILD_PATH)/%.o,$(ASRC) $(ASRC_$(DRIVER_NAME)))

# output device
OUTPUT = $(BIN_DIR)/$(DEVICE_FILE)

# the plipbox test tool
DEV_TEST_TOOL_NAME = dev_test_$(BUILD_TYPE)_$(CPUSUFFIX)
DEV_TEST_TOOL = $(BIN_DIR)/$(DEV_TEST_TOOL_NAME)
DEV_TEST_TOOL_OBJ = $(BUILD_PATH)/dev_test.o

# the udp test tool
UDP_TEST_TOOL_NAME = udp_test_$(BUILD_TYPE)_$(CPUSUFFIX)
UDP_TEST_TOOL = $(BIN_DIR)/$(UDP_TEST_TOOL_NAME)
UDP_TEST_TOOL_OBJ = $(BUILD_PATH)/udp_test.o

# add driver directory
VPATH = .:$(DEVICE_NAME)

# test dir
TEST_DIR ?= $(HOME)/amiga/shared/devs

# --- rules ---

.PHONY: dev_test udp_test tools libsize all opt dist clean clean_obj clean_dist dirs

all: dirs $(OUTPUT) tools

opt:
	$(MAKE) BUILD_TYPE=RELEASE

dist: clean_dist dirs
	$(MAKE) BUILD_TYPE=RELEASE CPUSUFFIX=000
	$(MAKE) BUILD_TYPE=RELEASE CPUSUFFIX=020
	$(MAKE) BUILD_TYPE=RELEASE CPUSUFFIX=040
	@rm -rf $(OBJ_DIR)

clean: clean_obj
	rm -f $(OUTPUT)

clean_obj:
	rm -f $(OBJS)

clean_dist:
	rm -rf $(BIN_DIR) $(OBJ_DIR)

dirs: $(BUILD_PATH) $(BIN_DIR)

test: dirs $(OUTPUT)
	cp $(OUTPUT) $(TEST_DIR)/plipbox.device

# --- compile ---
$(BUILD_PATH)/%.o: %.c
	$(CC) $(CFLAGS_$(BUILD_TYPE)) $(OBJ_NAME) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$@) $<

# --- assemble ---
$(BUILD_PATH)/%.o: %.asm
	$(AS) $(ASFLAGS) $(OBJ_NAME) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$@) $<

# --- link ---
$(OUTPUT): $(OBJS)
ifeq "$(LDFLAGS_HAS_MAP)" "1"
	$(LD) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$(OBJS)) \
	 	$(LDFLAGS_DEV) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@) \
		$(LDFLAGS_MAP) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@.map)$(LDFLAGS_MAP_POST)
else
	$(LD) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$(OBJS)) \
		$(LDFLAGS_DEV) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@)
endif

# --- dirs ---
$(BIN_DIR):
	mkdir $(BIN_DIR)

$(BUILD_PATH):
	mkdir -p $(BUILD_PATH)

# ----- tools -----
tools: dev_test udp_test

# --- dev_test tool ---
dev_test: $(DEV_TEST_TOOL)

$(DEV_TEST_TOOL): $(DEV_TEST_TOOL_OBJ)
	$(LD) $(LDFLAGS_PRE) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$<) \
	$(LDFLAGS_APP) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@)

# --- udp test tool ---
udp_test: $(UDP_TEST_TOOL)

$(UDP_TEST_TOOL): $(UDP_TEST_TOOL_OBJ)
	$(LD) $(LDFLAGS_PRE) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$<) \
	$(LDFLAGS_APP) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@)
