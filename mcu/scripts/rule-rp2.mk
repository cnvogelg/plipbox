# cmake-based pico build

.PHONY: ocd rebuild

build_arch: base_dir $(OUTPUT).elf

$(BASE_DIR)/Makefile: CMakeLists.txt
	@echo "  CMAKE  conf"
	$(HIDE)$(CMAKE) -B $(BASE_DIR) -S . \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DPICO_BOARD=pico_w \
		-DVERSION=$(VERSION) \
		-DVERSION_MAJ=$(VERSION_MAJ) \
		-DVERSION_MIN=$(VERSION_MIN) \
		-DBUILD_DATE=$(BUILD_DATE) \
		-DCONFIG_WIFI_SSID="$(CONFIG_WIFI_SSID)" \
		-DCONFIG_WIFI_PASS="$(CONFIG_WIFI_PASS)"

$(OUTPUT).elf: $(BASE_DIR)/Makefile
	@echo "  CMAKE  build"
	$(HIDE)$(CMAKE) --build $(BASE_DIR)

rebuild: $(BASE_DIR)/Makefile
	@echo "  CMAKE  build"
	$(HIDE)$(CMAKE) --build $(BASE_DIR)

prog_arch: $(OUTPUT).elf
	@echo "flash pico"

debug_arch: rebuild
	@echo "debug pico"
	$(HIDE)$(SCRIPT_DIR)/pico-gdb $(OUTPUT).elf

ocd:
	@echo "running OCD for debug probe"
	$(HIDE)$(SCRIPT_DIR)/pico-ocd
