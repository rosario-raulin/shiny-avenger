CPP=clang++
LD=clang++
INCLUDE_PATH=/usr/local/Cellar/boost/1.57.0/include
LINKER_PATH=/usr/local/Cellar/boost/1.57.0/lib

all: build link
fast: fast_build link

build:
	$(CPP) -std=c++11 -I$(INCLUDE_PATH) -O0 -Wall -Wpedantic -g -c src/*.cpp
	
fast_build:
	$(CPP) -std=c++11 -O3 -Wall -c src/*.cpp
	
link:
	$(LD) -L$(LINKER_PATH) -lboost_thread-mt -lboost_system -o pgrouping *.o

clean:
	rm -rf *.o pgrouping
