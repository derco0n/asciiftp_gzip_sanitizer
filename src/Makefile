CC=g++-10
CFLAGS=-std=c++17 -pthread
LDFLAGS=-lz

all: corrupter salvager

corrupter: corrupter.cpp
	$(CC) $(CFLAGS) corrupter.cpp -o corrupter

salvager: main.cpp parser.cpp
	$(CC) $(CFLAGS) main.cpp parser.cpp $(LDFLAGS) -o salvager

clean:
	rm -f corrupter salvager
