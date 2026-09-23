#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/lru_cache.h"

int main() {
    printf("=== Testing Basic Operations & Eviction ===\n");
    LRUCache *cache = lru_create(3, 16);

    lru_put(cache, "k1", "v1");
    lru_put(cache, "k2", "v2");
    lru_put(cache, "k3", "v3");

    lru_print_cache(cache);

    // Access k1 -> Moves k1 to MRU
    char *v1 = lru_get(cache, "k1");
    assert(v1 != NULL && strcmp(v1, "v1") == 0);
    free(v1);

    // Put k4 -> Capacity exceeded (3), evicts LRU element ("k2")
    lru_put(cache, "k4", "v4");

    char *evicted = lru_get(cache, "k2");
    assert(evicted == NULL);

    lru_print_cache(cache);

    printf("=== Testing Disk Persistence ===\n");
    lru_save_to_disk(cache, "cache_snapshot.bin");
    lru_destroy(cache);

    LRUCache *restored = lru_create(3, 16);
    lru_load_from_disk(restored, "cache_snapshot.bin");

    char *restored_val = lru_get(restored, "k4");
    assert(restored_val != NULL && strcmp(restored_val, "v4") == 0);
    free(restored_val);

    lru_print_cache(restored);
    lru_destroy(restored);

    remove("cache_snapshot.bin");
    printf("All basic and persistence tests passed successfully!\n");
    return 0;
}