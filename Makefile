
enumerator: enumerator.cpp *.h
	g++ -Iinclude -g -Wall -O3 -flto -Wno-reorder -std=c++14 -o enumerator enumerator.cpp -L. -lz3 -pthread

run: enumerator
	env LD_LIBRARY_PATH=. ./enumerator

enumerator2: enumerator2.cpp *.h
	g++ -Iinclude -g -Wall -O3 -flto -Wno-reorder -std=c++14 -o enumerator2 enumerator2.cpp -L. -lz3 -lprofiler -pthread

run2: enumerator2
	env LD_LIBRARY_PATH=. ./enumerator2 $(ARGS)

asm:
	g++ -Iinclude -g -Wall -O3 -flto -Wno-reorder -std=c++14 -save-temps -fverbose-asm enumerator2.cpp -L. -lz3 -lprofiler -pthread
