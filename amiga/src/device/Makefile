# Build plipbox.device

SCRIPT_DIR=../../scripts

include $(SCRIPT_DIR)/define.mk
include ../../../version.mk

# device setup
DEVICE_NAME = plipbox
DEVICE_VERSION = $(VERSION_MAJ)
DEVICE_REVISION = $(VERSION_MIN)
DEVICE_DATE = $(shell date '+%d.%m.%Y')
DEVICE_FILE = $(DEVICE_NAME).device
DEVICE_ID = $(DEVICE_NAME) $(DEVICE_VERSION).$(DEVICE_REVISION) ($(DEVICE_DATE))

CFLAGS += -DDEVICE_NAME="\"\\\"$(DEVICE_NAME).device\\\"\""
CFLAGS += -DDEVICE_VERSION=$(DEVICE_VERSION)
CFLAGS += -DDEVICE_REVISION=$(DEVICE_REVISION)
CFLAGS += -DDEVICE_ID="\"\\\"$(DEVICE_ID)\\\"\""
CFLAGS += -I$(DEVICE_NAME) -I$(DEVICE_NAME)/pbproto -I.

# generic source files
CSRC=rt.c device.c server.c track.c

# driver specific source files
CSRC += proto_env.c proto_atom.c proto_cmd.c proto_req.c timer.c pario.c proto_low_cfg.c
CSRC += hw.c
ASRC += proto_low.s pario_irq.s

# object files
OBJS+=$(patsubst %.c,$(OBJ_PATH)/%.o,$(CSRC) $(CSRC_$(DRIVER_NAME)))
OBJS+=$(patsubst %.s,$(OBJ_PATH)/%.o,$(ASRC) $(ASRC_$(DRIVER_NAME)))

# output device
DEVICE_PATH = $(BIN_PATH)/$(DEVICE_FILE)

# add driver directory
VPATH = .:$(DEVICE_NAME):plipbox/pbproto

# --- rules ---

.PHONY: all clean rtest test

all: dirs $(DEVICE_PATH)

include $(SCRIPT_DIR)/rule.mk

clean: clean_obj
	rm -f $(DEVICE_PATH)

test: dirs $(DEVICE_PATH)
	cp $(DEVICE_PATH) $(TEST_DIR)/$(DEVICE_FILE)

# --- compile ---
$(OBJ_PATH)/%.o: %.c
	@echo "  C   $<"
	$(H)$(CC) $(CFLAGS_$(BUILD_TYPE)) $(OBJ_NAME) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$@) $<

# --- assemble ---
$(OBJ_PATH)/%.o: %.s
	@echo "  ASM $<"
	$(H)$(AS) $(ASFLAGS) $(OBJ_NAME) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$@) $<

# --- link ---
$(DEVICE_PATH): $(OBJS)
	@echo "  LNK $@"
ifeq "$(LDFLAGS_HAS_MAP)" "1"
	$(H)$(LD) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$(OBJS)) \
	 	$(LDFLAGS_DEV) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@) \
		$(LDFLAGS_MAP) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@.map)$(LDFLAGS_MAP_POST)
else
	$(H)$(LD) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$(OBJS)) \
		$(LDFLAGS_DEV) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@)
endif
