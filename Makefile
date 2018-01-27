
all: enumerator.cpp
	g++ -g -Wall -O3 -Wno-reorder -std=c++11 -o enumerator enumerator.cpp -lz3 -pthread
