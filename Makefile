
all: enumerator.cpp
	g++ -g -Wall -O3 -flto -Wno-reorder -std=c++14 -o enumerator enumerator.cpp -lz3 -pthread