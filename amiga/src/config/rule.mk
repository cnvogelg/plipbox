.PHONY: opt compiler dist dist_all clean_obj clean_dist dirs

opt:
	@$(MAKE) BUILD_TYPE=RELEASE

compiler:
	$(H)$(MAKE) COMPILER=sc
	$(H)$(MAKE) COMPILER=vbcc
	$(H)$(MAKE) COMPILER=gcc

dist: clean_dist dirs
	$(H)$(MAKE) BUILD_TYPE=RELEASE CPUSUFFIX=000
	$(H)$(MAKE) BUILD_TYPE=RELEASE CPUSUFFIX=020
	$(H)$(MAKE) BUILD_TYPE=RELEASE CPUSUFFIX=040
	rm -rf $(OBJ_DIR)

dist_all:
	$(H)$(MAKE) COMPILER=sc dist
	$(H)$(MAKE) COMPILER=vbcc dist
	$(H)$(MAKE) COMPILER=gcc dist

clean_obj:
	rm -f $(OBJS)

clean_dist:
	rm -rf $(BIN_DIR) $(OBJ_DIR)

dirs: $(BUILD_PATH) $(BIN_DIR)

rtest:
	$(H)$(MAKE) BUILD_TYPE=RELEASE test

# --- dirs ---
$(BIN_DIR):
	$(H)mkdir $(BIN_DIR)

$(BUILD_PATH):
	$(H)mkdir -p $(BUILD_PATH)

