#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct Node {
    char *key;
    char *value;
    struct Node *prev;
    struct Node *next;
} Node;

typedef struct HashNode {
    Node *cache_node;
    struct HashNode *next;
} HashNode;

typedef struct LRUCache {
    size_t capacity;
    size_t size;
    size_t hash_capacity;

    Node *head;
    Node *tail;

    HashNode **hash_table;
    pthread_mutex_t lock;

} LRUCache;

LRUCache *lru_create(size_t capacity, size_t hash_capacity);
bool lru_put(LRUCache *cache, const char *key, const char *value);
char *lru_get(LRUCache *cache, const char *key);
bool lru_remove(LRUCache *cache, const char *key);
void lru_destroy(LRUCache *cache);


bool lru_save_to_disk(LRUCache *cache, const char *filename);
bool lru_load_from_disk(LRUCache *cache, const char *filename);
void lru_print_cache(LRUCache *cache);

#endif // LRU_CACHE_H