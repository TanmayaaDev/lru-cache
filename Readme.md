High-Performance Multi-Threaded LRU Cache Engine

A lightweight, concurrent, thread-safe In-Memory Key-Value LRU (Least Recently Used) Cache built in C with O(1) operations, mutex synchronization, and binary disk persistence.

Features

O(1) Cache Operations: Doubly-linked list coupled with a custom hash table (separate chaining) for constant-time read, insert, and eviction operations.

Thread-Safe Concurrency: Synchronized via pthread_mutex_t to handle highly concurrent multi-threaded read/write workloads without data corruption or race conditions.

Disk Persistence: Built-in dynamic binary serialization (lru_save_to_disk) and deserialization (lru_load_from_disk) routines to restore cache state across restarts.

Automatic LRU Eviction: Automatically purges the least recently accessed key-value pair when capacity limits are hit.

Directory Structure

lru-cache/
├── include/
│   └── lru_cache.h         # API declarations & data structure definitions
├── src/
│   └── lru_cache.c         # Core implementation (DLL, Hash Table, Locks, I/O)
├── tests/
│   └── test_lru.c          # Unit tests for core operations & file persistence
├── examples/
│   └── concurrent_benchmark.c  # Multi-threaded stress testing suite
└── Makefile                # Build automation
Getting Started
Prerequisites
C Compiler: gcc or clang (C99 standard or higher)

Build System: make

POSIX Threads: pthread library

Build & Run Tests
To compile the library, execute the unit test suite, and run the multi-threaded benchmark:

Bash
make clean
make test
API Reference

Initialization & Cleanup

LRUCache* lru_create(size_t capacity, size_t hash_capacity);
void lru_destroy(LRUCache *cache);

Core Operations

bool lru_put(LRUCache *cache, const char *key, const char *value);
char* lru_get(LRUCache *cache, const char *key);
bool lru_remove(LRUCache *cache, const char *key);

Persistence & Utility

bool lru_save_to_disk(LRUCache *cache, const char *filepath);
bool lru_load_from_disk(LRUCache *cache, const char *filepath);
void lru_print_cache(LRUCache *cache);

Benchmarks & Verification

The included benchmark spawns 4 worker threads concurrently performing 1,000 operations each (4,000 total synchronized transactions) against a shared cache instance, verifying data consistency and eviction correctness under heavy thread contention.