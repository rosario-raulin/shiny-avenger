CC=clang++
LD=clang++

all: build link

fast: fast_build link

build:
	$(CC) -std=c++11 -O0 -g -Wall -Wpedantic -c src/*.cpp
	
fast_build:
	$(CC) -std=c++11 -O2 -Wpedantic -c src/*.cpp
	
link:
	$(LD) -o tp *.o
