.PHONY: elf hex size_code size_data size_symbols size
.PHONY: build_arch prog_arch

build_arch: dirs hex lss size

elf: $(OUTPUT).elf $(OUTPUT).lss $(OUTPUT).sym
	@echo "  --- resulting elf ---"
	$(HIDE)$(SIZE) $(OUTPUT).elf -C --mcu=$(MCU)

hex: $(OUTPUT).hex

lss: $(OUTPUT).lss

size: size_code size_data

size_code: $(OUTPUT).elf
	@SIZE=`$(SIZE) -C $(OUTPUT).elf | grep Program | awk '{ print $$2 }'` ; \
	if [ $$SIZE -gt $(MAX_SIZE) ] ; then \
		echo "  $$SIZE >  $(MAX_SIZE) bytes: code TOO LARGE" ; exit 1 ; \
	else \
		echo "  $$SIZE <= $(MAX_SIZE) bytes: code ok" ; \
	fi

size_data: $(OUTPUT).elf
	@SIZE=`$(SIZE) -C $(OUTPUT).elf | fgrep Data | awk '{ print $$2 }'` ; \
	if [ $$SIZE -gt $(MAX_SRAM) ] ; then \
		echo "  $$SIZE >  $(MAX_SRAM) bytes: sram TOO LARGE" ; exit 1 ; \
	else \
		echo "  $$SIZE <= $(MAX_SRAM) bytes: sram ok" ; \
	fi

size_symbols: $(OUTPUT).elf
	@$(NM) --size-sort --print-size $(OUTPUT).elf | egrep ' [bBdD] '

# --- flash rules ---

check_prog:
	$(AVRDUDE) $(AVRDUDE_LDR_FLAGS) -U signature:r:sig.txt:h
	@echo -n " device signature: "
	@cat sig.txt
	@rm -f sig.txt

prog_arch: $(OUTPUT).hex size
	@echo "  --- programming flash ---"
	$(AVRDUDE) $(AVRDUDE_LDR_FLAGS) $(AVRDUDE_WRITE_FLASH)

read_fuse:
	$(AVRDUDE) $(AVRDUDE_LDR_FLAGS) -U lfuse:r:lfuse.txt:h -U hfuse:r:hfuse.txt:h
	@echo -n " lfuse: "
	@cat lfuse.txt
	@echo -n " hfuse: "
	@cat hfuse.txt
	@rm -f lfuse.txt hfuse.txt

# --- build rules ---

# final hex (flash) file from elf
%.hex: %.elf
	@echo "  HEX  $@"
	$(HIDE)$(OBJCOPY) -O $(FORMAT) -j .data -j .text $< $@

# finale eeprom file from elf
%.eep: %.elf
	@echo "  EEP  $@"
	$(HIDE)$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# extended listing file
%.lss: %.elf
	@echo "  LSS  $@"
	$(HIDE)$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo "  SYM  $@"
	$(HIDE)$(NM) -n $< > $@

# link
%.elf: $(OBJ)
	@echo "  LD   $@"
	$(HIDE)$(CC) $(LDFLAGS) $(OBJ) -o $@ $(LDLIBS)

# compile
$(OBJDIR)/%.o : %.c
	@echo "  CC   $<"
	$(HIDE)$(CC) -c $(CFLAGS) $< -o $@

# compile
$(OBJDIR)/%.o : %.S
	@echo "  AS   $<"
	$(HIDE)$(CC) -c $(ASFLAGS) $< -o $@
