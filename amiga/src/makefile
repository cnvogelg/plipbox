# gnu makefile to trigger build via vamos

BIN_DIR=../bin
OBJ_DIR=../obj

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
	$(VAMOS) $(VAMOS_OPTS) all_opt

dist: dirs
	$(VAMOS) $(VAMOS_OPTS) smake dist

clean:
	rm -rf $(BIN_DIR)
	rm -f *.no *.o *.opt???

dirs: $(BIN_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)
