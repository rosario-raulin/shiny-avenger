CPP=clang++
LD=clang++
INCLUDE_PATH=-I/usr/local/opt/boost149/include
LINKER_PATH=-L/usr/local/opt/boost149/lib

all: build link
fast: fast_build link

build:
	$(CPP) -I$(INCLUDE_PATH) -O0 -Wall -Wpedantic -g -c src/*.cpp
	
fast_build:
	$(CPP) -O3 -Wall -c src/*.cpp
	
link:
	$(LD) -L$(LINKER_PATH) -lboost_threads -o pgrouping *.o

clean:
	rm -rf *.o pgrouping
