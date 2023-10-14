include ../config/define.mk

# the plipbox tool
PLIPTOOL_NAME = pliptool_$(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
PLIPTOOL_PATH = $(BIN_DIR)/$(PLIPTOOL_NAME)
PLIPTOOL_SRC = pliptool.c sanadev.c
PLIPTOOL_OBJ = $(patsubst %.c,$(BUILD_PATH)/%.o,$(PLIPTOOL_SRC))

# the plipbox test tool
DEV_TEST_NAME = dev_test_$(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
DEV_TEST_PATH = $(BIN_DIR)/$(DEV_TEST_NAME)
DEV_TEST_SRC = dev_test.c atimer.c
DEV_TEST_OBJ = $(patsubst %.c,$(BUILD_PATH)/%.o,$(DEV_TEST_SRC))

# the udp test tool
UDP_TEST_NAME = udp_test_$(COMPILER)_$(BUILD_TYPE)_$(CPUSUFFIX)
UDP_TEST_PATH = $(BIN_DIR)/$(UDP_TEST_NAME)
UDP_TEST_SRC = udp_test.c
UDP_TEST_OBJ = $(patsubst %.c,$(BUILD_PATH)/%.o,$(UDP_TEST_SRC))

OBJS = $(DEV_TEST_BJ) $(UDP_TEST_OBJ) $(PLIPTOOL_OBJ)
TOOL_PATHS = $(PLIPTOOL_PATH) $(DEV_TEST_PATH) $(UDP_TEST_PATH)

# ----- tools -----
.PHONY: all $(TOOL_PATHS)

all: dirs $(TOOL_PATHS)

include ../config/rule.mk

clean: clean_obj
	rm -f $(DEV_TEST_PATH) $(UDP_TEST_PATH) $(PLIPTOOL_PATH)

test: dirs $(DEV_TEST_PATH) $(UDP_TEST_PATH) $(PLIPTOOL_PATH)
	cp $(DEV_TEST_PATH) $(TEST_DIR)/dev_test
	cp $(UDP_TEST_PATH) $(TEST_DIR)/udp_test
	cp $(PLIPTOOL_PATH) $(TEST_DIR)/pliptool

# --- pliptool ---
pliptool: $(PLIPTOOL_PATH)

$(PLIPTOOL_PATH): $(PLIPTOOL_OBJ)
	@echo "  LNK $@"
	$(H)$(LD) $(LDFLAGS_PRE) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$(PLIPTOOL_OBJ)) \
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
