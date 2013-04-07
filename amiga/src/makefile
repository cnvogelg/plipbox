# gnu makefile to trigger build via vamos

BIN_DIR=../bin
OBJ_DIR=../obj
INSTALL_DIR=$(HOME)/amiga/shared/test

# where does the amiga files reside?
# expects the following dirs:
# wb310          - HD install or workbench 3.1
# sc             - install directory of SAS C 6.58 compiler
# AmiTCP-SDK-4.3 - AmiTCP SDK installation
AMIGA_DIR?=$(HOME)/amiga/shared
export AMIGA_DIR

# config vamos
VAMOS=vamos
VAMOS_OPTS=-c vamosrc

all: dirs
	$(VAMOS) $(VAMOS_OPTS) smake all

all_opt: dirs
	$(VAMOS) $(VAMOS_OPTS) smake all_opt

dist: dirs release clean_obj

release: dirs
	$(VAMOS) $(VAMOS_OPTS) smake release

install: release
	cp $(BIN_DIR)/plipbox.device.??? $(INSTALL_DIR)/
	cp plipbox.config $(INSTALL_DIR)/

clean: clean_obj
	rm -rf $(BIN_DIR)

clean_obj:
	rm -f *.no *.o *.opt???

clean_dist: clean

dirs: $(BIN_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)
