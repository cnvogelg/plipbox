.PHONY: all clean test rtest

# ----- rules ---
all:
	@make -C device all
	@make -C tools all

dist:
	@make -C device dist
	@make -C tools dist

clean:
	@make -C device clean
	@make -C tools clean

clean_dist:
	@make -C device clean_dist
	@make -C tools clean_dist

test:
	@make -C device test
	@make -C tools test
	@make -C test test

rtest:
	@make -C device rtest
	@make -C tools rtest
	@make -C test test
