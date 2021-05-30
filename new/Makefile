

main: main.cpp
	g++ -Iinclude -g -Wall -O3 -flto -Wno-reorder -std=c++17 -o main main.cpp -L. -lz3 -pthread

nestest: main
	env LD_LIBRARY_PATH=. ./main nestest.nes | tee test.txt
	git diff --no-index test.txt golden-new.txt

test: main
	env LD_LIBRARY_PATH=. ./main test_roms/01-basics.nes
	env LD_LIBRARY_PATH=. ./main test_roms/02-implied.nes
	env LD_LIBRARY_PATH=. ./main test_roms/03-immediate.nes
	env LD_LIBRARY_PATH=. ./main test_roms/04-zero_page.nes
	env LD_LIBRARY_PATH=. ./main test_roms/05-zp_xy.nes
	env LD_LIBRARY_PATH=. ./main test_roms/06-absolute.nes
	env LD_LIBRARY_PATH=. ./main test_roms/07-abs_xy.nes
	env LD_LIBRARY_PATH=. ./main test_roms/08-ind_x.nes
	env LD_LIBRARY_PATH=. ./main test_roms/09-ind_y.nes
	env LD_LIBRARY_PATH=. ./main test_roms/10-branches.nes
	env LD_LIBRARY_PATH=. ./main test_roms/11-stack.nes
	env LD_LIBRARY_PATH=. ./main test_roms/12-jmp_jsr.nes
	env LD_LIBRARY_PATH=. ./main test_roms/13-rts.nes
	env LD_LIBRARY_PATH=. ./main test_roms/14-rti.nes
	env LD_LIBRARY_PATH=. ./main test_roms/15-brk.nes
	env LD_LIBRARY_PATH=. ./main test_roms/16-special.nes
