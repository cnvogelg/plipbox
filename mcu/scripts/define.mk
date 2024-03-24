# defines.mk
# common defines for all boards

OS=$(shell uname -s)

BUILD_DATE = $(shell date '+%Y%m%d')
BUILD_TYPE ?= DEBUG

# basename of output
BASE_NAME = $(BOARD)-$(ARCH)-$(MACH)-$(BUILD_TYPE)

# build directory
BUILD_DIR = ../build/
BASE_DIR = $(BUILD_DIR)/$(BASE_NAME)

OBJDIR = $(BASE_DIR)/obj
DEPDIR = $(BASE_DIR)/dep
BINDIR = $(BASE_DIR)/bin

# project
OUTPUT = $(BINDIR)/$(PROJECT)

# dist dir
DISTDIR = ../dist

# define V to see compile output
ifdef V
HIDE=
else
HIDE=@
endif
