# cmake-based pico build

build_arch: base_dir $(BASE_DIR)/Makefile
	$(HIDE)$(CMAKE) --build $(BASE_DIR)

$(BASE_DIR)/Makefile: CMakeLists.txt
	@echo "  CMAKE"
	$(HIDE)$(CMAKE) -B $(BASE_DIR) -S . \
		-DVERSION=$(VERSION) \
		-DVERSION_MAJ=$(VERSION_MAJ) \
		-DVERSION_MIN=$(VERSION_MIN) \
		-DBUILD_DATE=$(BUILD_DATE)

prog_arch:
	@echo "flash pico"
