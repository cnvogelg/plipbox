SCRIPT_DIR=../../scripts

include $(SCRIPT_DIR)/define.mk

# lib files
LIBTOOL_SRC = atimer.c param.c param_tag.c pktbuf.c
LIBTOOL_SRC += plipbox_cmd.c sanadev.c
LIBTOOL_SRC += plipbox_req_param.c plipbox_req_nic.c plipbox_req_mode.c
LIBTOOL_BASE = libtool
LIBTOOL_NAME = $(LIBTOOL_BASE)$(LIB_EXT)
LIBTOOL_PATH = $(OBJ_PATH)/$(LIBTOOL_NAME)
LIBTOOL_OBJ = $(patsubst %.c,$(OBJ_PATH)/%.o,$(LIBTOOL_SRC))

# add lib
CFLAGS += -Ilib
VPATH = lib
LDFLAGS += -L$(OBJ_PATH) -l$(LIBTOOL_BASE)

# define tools
$(eval $(call def_tool,PLIPTOOL,pliptool))
$(eval $(call def_tool,PLIPBENCH,plipbench,bench.c))
$(eval $(call def_tool,PLIPEVENT,plipevent))
$(eval $(call def_tool,PLIPLINK,pliplink))
$(eval $(call def_tool,UDP_TEST,udp_test))

# ----- tools -----
.PHONY: all libtool $(TOOLS)

all: dirs $(TOOLS)

include $(SCRIPT_DIR)/rule.mk

clean: $(patsubst %,clean-%,$(TOOLS))

test: dirs $(patsubst %,test-%,$(TOOLS))

# --- libtool ---
libtool: $(LIBTOOL_PATH)

$(LIBTOOL_PATH): $(LIBTOOL_OBJ)
	@echo "  AR  $@"
	$(H)$(AR) $(AR_OUT)$@ $(LIBTOOL_OBJ)

# --- tools ---
$(eval $(call add_tool,PLIPTOOL,pliptool))
$(eval $(call add_tool,PLIPBENCH,plipbench))
$(eval $(call add_tool,PLIPEVENT,plipevent))
$(eval $(call add_tool,PLIPLINK,pliplink))
$(eval $(call add_tool,UDP_TEST,udp_test))

# --- compile ---
$(OBJ_PATH)/%.o: %.c
	@echo "  C   $<"
	$(H)$(CC) $(CFLAGS_$(BUILD_TYPE)) $(OBJ_NAME) $(subst $(OBJ_DIR),$($(PREFIX)OBJ_DIR),$@) $<
