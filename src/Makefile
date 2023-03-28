CC=g++-10
CFLAGS=-std=c++17 -pthread -g
LDFLAGS=-lz

all: corrupter salvager

corrupter: corrupter.cpp
	$(CC) $(CFLAGS) corrupter.cpp -o corrupter

salvager: main.cpp parser.cpp
	$(CC) $(CFLAGS) main.cpp parser.cpp uint256_t/uint128_t/uint128_t.cpp uint256_t/uint256_t.cpp $(LDFLAGS) -o salvager_dev

clean:
	rm -f corrupter salvager
