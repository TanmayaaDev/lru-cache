#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../include/lru_cache.h"

#define NUM_THREADS 4
#define OPS_PER_THREAD 1000

typedef struct {
    LRUCache *cache;
    int thread_id;
} ThreadArg;

void* worker_task(void *arg) {
    ThreadArg *targ = (ThreadArg*)arg;
    char key[32];
    char val[32];

    for (int i = 0; i < OPS_PER_THREAD; i++) {
        snprintf(key, sizeof(key), "t%d_k%d", targ->thread_id, i % 10);
        snprintf(val, sizeof(val), "val_%d", i);

        lru_put(targ->cache, key, val);

        char *fetched = lru_get(targ->cache, key);
        if (fetched) free(fetched);
    }
    return NULL;
}

int main() {
    printf("=== Multithreaded Concurrency Benchmark ===\n");
    LRUCache *cache = lru_create(20, 64);

    pthread_t threads[NUM_THREADS];
    ThreadArg args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].cache = cache;
        args[i].thread_id = i;
        pthread_create(&threads[i], NULL, worker_task, &args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Executed %d concurrent thread operations successfully!\n", NUM_THREADS * OPS_PER_THREAD);
    lru_print_cache(cache);

    lru_destroy(cache);
    return 0;
}