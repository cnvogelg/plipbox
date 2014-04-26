# gnu makefile to trigger build via vamos

BIN_DIR=../bin
OBJ_DIR=../obj

# mapped dir in vamos
AMI_OBJ_DIR=base:obj
AMI_BIN_DIR=base:bin

# where does the amiga files reside?
# expects the following dirs:
# wb310          - HD install of workbench 3.1
# sc             - install directory of SAS C 6.58 compiler
# AmiTCP-SDK-4.3 - AmiTCP SDK installation
AMIGA_DIR?=$(HOME)/amiga/shared
export AMIGA_DIR

# config vamos
VAMOS=vamos
VAMOS_OPTS=-c vamosrc
RUN=$(VAMOS) $(VAMOS_OPTS)

# SAS C tools
LD          = $(RUN) sc:c/slink
CC          = $(RUN) sc:c/sc
AS          = $(RUN) sc:c/sc

# SAS C flags
LIBS        = LIB LIB:amiga.lib LIB lib:sc.lib LIB lib:debug.lib
CINCLUDES   = INCDIR="include:" INCDIR="netinclude:"
ASMINCLUDES = $(CINCLUDES)
CPUSUFFIX   = 000
CPUCCOPT    = CPU=68$(CPUSUFFIX)
BUILD_TYPE  = DEBUG

# shared flags
CFLAGS_COMMON = $(CINCLUDES) NOMULTIPLEINCLUDES ERRREXX COMMENTNEST \
                NOSTKCHK NOCHKABORT \
                SMALLCODE SMALLDATA VERBOSE NOICONS STRICT ANSI \
				$(CPUCCOPT)

# build type flags
CFLAGS_DEBUG = $(CFLAGS_COMMON) 
CFLAGS_RELEASE = $(CFLAGS_COMMON) \
                 PARAMETERS=REGISTERS OPT OPTTIME OPTINLINE\
                 OPTSCHEDULE STRINGMERGE STRUCTUREEQUIVALENCE

# assembler
ASMFLAGS = $(ASMINCLUDES) VERBOSE

# linker
LDFLAGS_DEBUG    = NOICONS SC  $(LIBS) TO
LDFLAGS_RELEASE  = NOICONS SC SD ND $(LIBS) TO

# output directory
BUILD_DIR = build_$(BUILD_TYPE)_$(CPUSUFFIX)
BUILD_PATH = $(OBJ_DIR)/$(BUILD_DIR)

# device file
DEVICE_NAME = plipbox
DEVICE_FILE = $(DEVICE_NAME).device_$(BUILD_TYPE)_$(CPUSUFFIX)

# source files
CSRC=device.c hw.c server.c track.c
ASRC=rt.asm hwpar.asm

# object files
OBJS:=$(patsubst %.c,$(BUILD_PATH)/%.o,$(CSRC))
OBJS+=$(patsubst %.asm,$(BUILD_PATH)/%.o,$(ASRC))

# output device
OUTPUT = $(BIN_DIR)/$(DEVICE_FILE)

# --- rules ---

all: dirs $(OUTPUT)

opt:
	$(MAKE) BUILD_TYPE=RELEASE

dist: dirs
	$(MAKE) BUILD_TYPE=RELEASE CPUSUFFIX=000
	$(MAKE) BUILD_TYPE=RELEASE CPUSUFFIX=020
	$(MAKE) BUILD_TYPE=RELEASE CPUSUFFIX=040

clean: clean_obj
	rm -f $(OUTPUT)

clean_obj:
	rm -f $(OBJS)

clean_dist:
	rm -rf $(BIN_DIR) $(OBJ_DIR)

dirs: $(BUILD_PATH) $(BIN_DIR)

# --- compile ---
$(BUILD_PATH)/%.o: %.c
	$(CC) $(CFLAGS_$(BUILD_TYPE)) OBJECTNAME $(subst $(OBJ_DIR),$(AMI_OBJ_DIR),$@) $<

# --- assemble ---
$(BUILD_PATH)/%.o: %.asm
	$(AS) $(ASMFLAGS) OBJECTNAME $(subst $(OBJ_DIR),$(AMI_OBJ_DIR),$@) $<

# --- link ---
$(OUTPUT): $(OBJS)
	$(LD) $(subst $(OBJ_DIR),$(AMI_OBJ_DIR),$(OBJS)) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$(AMI_BIN_DIR),$@)

# --- dirs ---
$(BIN_DIR):
	mkdir $(BIN_DIR)

$(BUILD_PATH):
	mkdir -p $(BUILD_PATH)
