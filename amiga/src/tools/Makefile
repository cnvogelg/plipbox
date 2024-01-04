include ../config/define.mk

# pliptool
PLIPTOOL_NAME = pliptool_$(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
PLIPTOOL_PATH = $(BIN_DIR)/$(PLIPTOOL_NAME)
PLIPTOOL_SRC = pliptool.c
PLIPTOOL_SRC += sanadev.c param.c plipbox_cmd.c
PLIPTOOL_OBJ = $(patsubst %.c,$(BUILD_PATH)/%.o,$(PLIPTOOL_SRC))

# plipbench
PLIPBENCH_NAME = plipbench_$(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
PLIPBENCH_PATH = $(BIN_DIR)/$(PLIPBENCH_NAME)
PLIPBENCH_SRC = plipbench.c bench.c pktbuf.c
PLIPBENCH_SRC += sanadev.c param.c param_tag.c plipbox_cmd.c atimer.c
PLIPBENCH_OBJ = $(patsubst %.c,$(BUILD_PATH)/%.o,$(PLIPBENCH_SRC))

# plipevent
PLIPEVENT_NAME = plipevent_$(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
PLIPEVENT_PATH = $(BIN_DIR)/$(PLIPEVENT_NAME)
PLIPEVENT_SRC = plipevent.c
PLIPEVENT_SRC += sanadev.c atimer.c
PLIPEVENT_OBJ = $(patsubst %.c,$(BUILD_PATH)/%.o,$(PLIPEVENT_SRC))

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
TOOL_PATHS = $(PLIPTOOL_PATH) $(PLIPBENCH_PATH) $(PLIPEVENT_PATH)
TOOL_PATHS += $(DEV_TEST_PATH) $(UDP_TEST_PATH)

# ----- tools -----
.PHONY: all $(TOOL_PATHS)

all: dirs $(TOOL_PATHS)

include ../config/rule.mk

clean: clean_obj
	rm -f $(TOOL_PATHS)

test: dirs $(TOOL_PATHS)
#	cp $(DEV_TEST_PATH) $(TEST_DIR)/dev_test
	cp $(UDP_TEST_PATH) $(TEST_DIR)/udp_test
	cp $(PLIPTOOL_PATH) $(TEST_DIR)/pliptool
	cp $(PLIPBENCH_PATH) $(TEST_DIR)/plipbench
	cp $(PLIPEVENT_PATH) $(TEST_DIR)/plipevent

# --- pliptool ---
pliptool: $(PLIPTOOL_PATH)

$(PLIPTOOL_PATH): $(PLIPTOOL_OBJ)
	@echo "  LNK $@"
	$(H)$(LD) $(LDFLAGS_PRE) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$(PLIPTOOL_OBJ)) \
	$(LDFLAGS_APP) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@)

# --- plipbench ---
plipbench: $(PLIPBENCH_PATH)

$(PLIPBENCH_PATH): $(PLIPBENCH_OBJ)
	@echo "  LNK $@"
	$(H)$(LD) $(LDFLAGS_PRE) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$(PLIPBENCH_OBJ)) \
	$(LDFLAGS_APP) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@)

# --- plipevent ---
plipevent: $(PLIPEVENT_PATH)

$(PLIPEVENT_PATH): $(PLIPEVENT_OBJ)
	@echo "  LNK $@"
	$(H)$(LD) $(LDFLAGS_PRE) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$(PLIPEVENT_OBJ)) \
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
