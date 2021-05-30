

main: main.cpp
	g++ -Iinclude -g -Wall -O3 -flto -Wno-reorder -std=c++17 -o main main.cpp -L. -lz3 -pthread

nestest: main
	env LD_LIBRARY_PATH=. ./main nestest.nes | tee test.txt
	git diff --no-index test.txt golden-new.txt

test: main
	env LD_LIBRARY_PATH=. ./main test_roms/01-basics.nes > out/01.txt &
	env LD_LIBRARY_PATH=. ./main test_roms/02-implied.nes > out/02.txt &
	env LD_LIBRARY_PATH=. ./main test_roms/03-immediate.nes > out/03.txt &
	env LD_LIBRARY_PATH=. ./main test_roms/04-zero_page.nes > out/04.txt &
	env LD_LIBRARY_PATH=. ./main test_roms/05-zp_xy.nes > out/05.txt &
	env LD_LIBRARY_PATH=. ./main test_roms/06-absolute.nes > out/06.txt &
	env LD_LIBRARY_PATH=. ./main test_roms/07-abs_xy.nes > out/07.txt &
	env LD_LIBRARY_PATH=. ./main test_roms/08-ind_x.nes > out/08.txt
	env LD_LIBRARY_PATH=. ./main test_roms/09-ind_y.nes > out/09.txt &
	env LD_LIBRARY_PATH=. ./main test_roms/10-branches.nes > out/10.txt &
	env LD_LIBRARY_PATH=. ./main test_roms/11-stack.nes > out/11.txt &
	env LD_LIBRARY_PATH=. ./main test_roms/12-jmp_jsr.nes > out/12.txt &
	env LD_LIBRARY_PATH=. ./main test_roms/13-rts.nes > out/13.txt &
	env LD_LIBRARY_PATH=. ./main test_roms/14-rti.nes > out/14.txt &
	env LD_LIBRARY_PATH=. ./main test_roms/15-brk.nes > out/15.txt &
	env LD_LIBRARY_PATH=. ./main test_roms/16-special.nes > out/16.txt
