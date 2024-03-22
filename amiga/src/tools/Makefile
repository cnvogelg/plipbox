include ../config/define.mk

# lib files
LIBTOOL_SRC = atimer.c param.c param_tag.c pktbuf.c
LIBTOOL_SRC += plipbox_cmd.c plipbox_req.c sanadev.c
LIBTOOL_BASE = libtool_$(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
LIBTOOL_NAME = $(LIBTOOL_BASE)$(LIB_EXT)
LIBTOOL_PATH = $(BIN_DIR)/$(LIBTOOL_NAME)
LIBTOOL_OBJ = $(patsubst %.c,$(BUILD_PATH)/%.o,$(LIBTOOL_SRC))

# add lib
CFLAGS += -Ilib
VPATH = lib
LDFLAGS += -L$(BIN_DIR) -l$(LIBTOOL_BASE)

# pliptool
PLIPTOOL_NAME = pliptool_$(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
PLIPTOOL_PATH = $(BIN_DIR)/$(PLIPTOOL_NAME)
PLIPTOOL_SRC = pliptool.c
PLIPTOOL_SRC += $(LIB_SRC)
PLIPTOOL_OBJ = $(patsubst %.c,$(BUILD_PATH)/%.o,$(PLIPTOOL_SRC))

# plipbench
PLIPBENCH_NAME = plipbench_$(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
PLIPBENCH_PATH = $(BIN_DIR)/$(PLIPBENCH_NAME)
PLIPBENCH_SRC = plipbench.c bench.c
PLIPBENCH_SRC += $(LIB_SRC)
PLIPBENCH_OBJ = $(patsubst %.c,$(BUILD_PATH)/%.o,$(PLIPBENCH_SRC))

# plipevent
PLIPEVENT_NAME = plipevent_$(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
PLIPEVENT_PATH = $(BIN_DIR)/$(PLIPEVENT_NAME)
PLIPEVENT_SRC = plipevent.c
PLIPEVENT_SRC += $(LIB_SRC)
PLIPEVENT_OBJ = $(patsubst %.c,$(BUILD_PATH)/%.o,$(PLIPEVENT_SRC))

# pliplink
PLIPLINK_NAME = pliplink_$(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
PLIPLINK_PATH = $(BIN_DIR)/$(PLIPLINK_NAME)
PLIPLINK_SRC = pliplink.c
PLIPLINK_SRC += $(LIB_SRC)
PLIPLINK_OBJ = $(patsubst %.c,$(BUILD_PATH)/%.o,$(PLIPLINK_SRC))

# the plipbox test tool
#DEV_TEST_NAME = dev_test_$(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
#DEV_TEST_PATH = $(BIN_DIR)/$(DEV_TEST_NAME)
#DEV_TEST_SRC = dev_test.c atimer.c
#DEV_TEST_OBJ = $(patsubst %.c,$(BUILD_PATH)/%.o,$(DEV_TEST_SRC))

# the udp test tool
UDP_TEST_NAME = udp_test_$(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
UDP_TEST_PATH = $(BIN_DIR)/$(UDP_TEST_NAME)
UDP_TEST_SRC = udp_test.c
UDP_TEST_OBJ = $(patsubst %.c,$(BUILD_PATH)/%.o,$(UDP_TEST_SRC))

OBJS = $(DEV_TEST_OBJ) $(UDP_TEST_OBJ) $(PLIPTOOL_OBJ) $(PLIPBENCH_OBJ) $(PLIPEVENT_OBJ)
OBJS += $(LIBTOOL_OBJ)
TOOL_PATHS = $(PLIPTOOL_PATH) $(PLIPBENCH_PATH) $(PLIPEVENT_PATH) $(PLIPLINK_PATH)
TOOL_PATHS += $(DEV_TEST_PATH) $(UDP_TEST_PATH)

# ----- tools -----
.PHONY: all libtool $(TOOL_PATHS)

all: dirs $(TOOL_PATHS)

include ../config/rule.mk

clean: clean_obj
	rm -f $(TOOL_PATHS) $(LIBTOOL_PATH)

test: dirs $(TOOL_PATHS)
#	cp $(DEV_TEST_PATH) $(TEST_DIR)/dev_test
	cp $(UDP_TEST_PATH) $(TEST_DIR)/udp_test
	cp $(PLIPTOOL_PATH) $(TEST_DIR)/pliptool
	cp $(PLIPBENCH_PATH) $(TEST_DIR)/plipbench
	cp $(PLIPEVENT_PATH) $(TEST_DIR)/plipevent
	cp $(PLIPLINK_PATH) $(TEST_DIR)/pliplink

# --- libtool ---
libtool: $(LIBTOOL_PATH)

$(LIBTOOL_PATH): $(LIBTOOL_OBJ)
	@echo "  AR  $@"
	$(H)$(AR) $(AR_OUT)$@ $(LIBTOOL_OBJ)

# --- pliptool ---
pliptool: $(PLIPTOOL_PATH)

$(PLIPTOOL_PATH): $(PLIPTOOL_OBJ) $(LIBTOOL_PATH)
	@echo "  LNK $@"
	$(H)$(LD) $(LDFLAGS_PRE) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$(PLIPTOOL_OBJ)) \
	$(LDFLAGS_APP) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@)

# --- plipbench ---
plipbench: $(PLIPBENCH_PATH)

$(PLIPBENCH_PATH): $(PLIPBENCH_OBJ) $(LIBTOOL_PATH)
	@echo "  LNK $@"
	$(H)$(LD) $(LDFLAGS_PRE) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$(PLIPBENCH_OBJ)) \
	$(LDFLAGS_APP) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@)

# --- plipevent ---
plipevent: $(PLIPEVENT_PATH)

$(PLIPEVENT_PATH): $(PLIPEVENT_OBJ) $(LIBTOOL_PATH)
	@echo "  LNK $@"
	$(H)$(LD) $(LDFLAGS_PRE) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$(PLIPEVENT_OBJ)) \
	$(LDFLAGS_APP) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@)

# --- pliplink ---
pliplink: $(PLIPLINK_PATH)

$(PLIPLINK_PATH): $(PLIPLINK_OBJ) $(LIBTOOL_PATH)
	@echo "  LNK $@"
	$(H)$(LD) $(LDFLAGS_PRE) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$(PLIPLINK_OBJ)) \
	$(LDFLAGS_APP) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@)

# --- dev_test tool ---
dev_test: $(DEV_TEST_PATH)

$(DEV_TEST_PATH): $(DEV_TEST_OBJ)
	@echo "  LNK $@"
	$(H)$(LD) $(LDFLAGS_PRE) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$(DEV_TEST_OBJ)) \
	$(LDFLAGS_APP) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@)

# --- udp test tool ---
udp_test: $(UDP_TEST_PATH)

$(UDP_TEST_PATH): $(UDP_TEST_OBJ)
	@echo "  LNK $@"
	$(H)$(LD) $(LDFLAGS_PRE) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$(UDP_TEST_OBJ)) \
	$(LDFLAGS_APP) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@)

# --- compile ---
$(BUILD_PATH)/%.o: %.c
	@echo "  C   $<"
	$(H)$(CC) $(CFLAGS_$(BUILD_TYPE)) $(OBJ_NAME) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$@) $<
