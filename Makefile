CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude -pthread

SRC = src/lru_cache.c
OBJ = $(SRC:.c=.o)

all: tests/test_lru examples/concurrent_benchmark

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

tests/test_lru: src/lru_cache.o tests/test_lru.c
	$(CC) $(CFLAGS) $^ -o $@

examples/concurrent_benchmark: src/lru_cache.o examples/concurrent_benchmark.c
	$(CC) $(CFLAGS) $^ -o $@

test: all
	@echo "--- Running Unit Tests & Multithreaded Benchmark ---"
	./tests/test_lru
	./examples/concurrent_benchmark

clean:
	rm -f src/*.o tests/test_lru examples/concurrent_benchmark cache_snapshot.bin

.PHONY: all test clean