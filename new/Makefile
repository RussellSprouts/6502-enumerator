

main: main.cpp operations.h nstdint.h zstdint.h initial_state.h config.h special_immediate.h
	g++ -Iinclude -g -Wall -O3 -flto -Wno-reorder -std=c++20 -o main main.cpp -L. -lz3 -pthread

nestest: main
	env LD_LIBRARY_PATH=. ./main nestest nestest.nes > test.txt
	git diff --no-index test.txt golden-new.txt

compare: main
	env LD_LIBRARY_PATH=. ./main compare

test: main
	env LD_LIBRARY_PATH=. ./main test test_roms/01-basics.nes > out/01.txt

SRCS = \
	test_roms/6502_65c02_functional_tests/6502_decimal_test.ca65 \
	test_roms/6502_65c02_functional_tests/6502_functional_test.ca65 \
	test_roms/6502_65c02_functional_tests/65C02_extended_opcodes_test.ca65

OBJS = $(SRCS:%.ca65=%.bin)

functional_tests: $(OBJS)

%.bin: %.ca65
	ca65 -l $*.lst $<
	ld65 $*.o -o $@ -m $*.map -C test_roms/6502_65c02_functional_tests/example.cfg

