.PHONY: all base_dir dirs hdr clean clean_dist
.PHONY: build rbuild prog rprog build_all rbuild_all

all: build

help:
	@echo "  on system: $(OS)"
	@echo "  supported boards: $(ALL_BOARDS)"
	@echo
	@echo "build [BOARD=<board>]  build board (debug)"
	@echo "prog  [BOARD=<board>]  program/flash board (debug)"
	@echo
	@echo "rbuild [BOARD=<board>] build board (release)"
	@echo "rprog  [BOARD=<board>] program/flash board (release)"
	@echo
	@echo "build_all              build all boards: debug"
	@echo "rbuild_all             build all boards: release"
	@echo
	@echo "dist                   build distribution"
	@echo "snap                   build snap shot"
	@echo
	@echo "clean                  clean current board build"
	@echo "clean_all              clean all builds"
	@echo "clean_dist             clean all builds and distribution files"

build: hdr build_arch

rbuild:
	$(HIDE)$(MAKE) BUILD_TYPE=RELEASE

clean:
	rm -rf $(BASE_DIR)

clean_all:
	rm -rf $(BUILD_DIR)

clean_dist: clean_all
	rm -rf $(DISTDIR)

build_all: clean_all
	@for a in $(DIST_BOARDS) ; do \
		$(MAKE) build BOARD=$$a ;\
		if [ $$? != 0 ]; then exit 1 ; fi ; \
	done

rbuild_all: clean_all
	@for a in $(DIST_BOARDS) ; do \
		$(MAKE) build BOARD=$$a BUILD_TYPE=RELEASE ;\
		if [ $$? != 0 ]; then exit 1 ; fi ; \
	done

dist: rbuild_all
	@echo "dist done"

snap: rbuild_all
	@echo "snap done"

prog: prog_arch

rprog:
	$(HIDE)$(MAKE) BUILD_TYPE=RELEASE prog

.PRECIOUS: $(OBJ)

# helper rules

base_dir:
	@if [ ! -d $(BASE_DIR) ]; then mkdir -p $(BASE_DIR); fi

dirs:
	@if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi
	@if [ ! -d $(DEPDIR) ]; then mkdir -p $(DEPDIR); fi
	@if [ ! -d $(BINDIR) ]; then mkdir -p $(BINDIR); fi

hdr:
	@echo "--- building BOARD=$(BOARD) ARCH=$(ARCH) MACH=$(MACH) ---"

