include ../config/define.mk

# the plipbox test tool
DEV_TEST_TOOL_NAME = dev_test_$(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
DEV_TEST_TOOL = $(BIN_DIR)/$(DEV_TEST_TOOL_NAME)
DEV_TEST_TOOL_SRC = dev_test.c atimer.c
DEV_TEST_TOOL_OBJ = $(patsubst %.c,$(BUILD_PATH)/%.o,$(DEV_TEST_TOOL_SRC))

# the udp test tool
UDP_TEST_TOOL_NAME = udp_test_$(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
UDP_TEST_TOOL = $(BIN_DIR)/$(UDP_TEST_TOOL_NAME)
UDP_TEST_TOOL_OBJ = $(BUILD_PATH)/udp_test.o

OBJS = $(DEV_TEST_TOOL_OBJ) $(UDP_TEST_TOOL_OBJ)

# ----- tools -----
.PHONY: all dev_test udp_test

all: dirs dev_test udp_test

include ../config/rule.mk

clean: clean_obj
	rm -f $(DEV_TEST_TOOL) $(UDP_TEST_TOOL)

test: dirs $(DEV_TEST_TOOL) $(UDP_TEST_TOOL)
	cp $(DEV_TEST_TOOL) $(TEST_DIR)/dev_test
	cp $(UDP_TEST_TOOL) $(TEST_DIR)/udp_test

# --- dev_test tool ---
dev_test: $(DEV_TEST_TOOL)

$(DEV_TEST_TOOL): $(DEV_TEST_TOOL_OBJ)
	@echo "  LNK $@"
	$(H)$(LD) $(LDFLAGS_PRE) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$(DEV_TEST_TOOL_OBJ)) \
	$(LDFLAGS_APP) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@)

# --- udp test tool ---
udp_test: $(UDP_TEST_TOOL)

$(UDP_TEST_TOOL): $(UDP_TEST_TOOL_OBJ)
	@echo "  LNK $@"
	$(H)$(LD) $(LDFLAGS_PRE) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$<) \
	$(LDFLAGS_APP) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$($(PREFIX)BIN_DIR),$@)

# --- compile ---
$(BUILD_PATH)/%.o: %.c
	@echo "  C   $<"
	$(H)$(CC) $(CFLAGS_$(BUILD_TYPE)) $(OBJ_NAME) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$@) $<
