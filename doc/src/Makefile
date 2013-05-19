# Makefile for documentation

# i use py32-markdown2 from MacPorts
MD_TOOL=markdown2-3.2
DST_DIR=../html
SRC=amiga.md hardware.md firmware.md python.md intro.md
DST=$(patsubst %.md,$(DST_DIR)/%.html,$(SRC))

dist: $(DST_DIR) $(DST)

clean:
	rm -rf $(DST_DIR)

clean_dist: clean

show: dist
	open $(DST)

$(DST_DIR):
	mkdir $(DST_DIR)

$(DST_DIR)/%.html: %.md
	$(MD_TOOL) $< > $@

