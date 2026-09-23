#include "lru_cache.h"

static unsigned int hash_string(const char *str, size_t hash_capacity) {
    unsigned long hash = 5381;
    int c;
    while((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return (unsigned int)(hash % hash_capacity);
}

static Node* create_node(const char *key, const char *value) {
    Node *node = (Node*)malloc(sizeof(Node));
    node->key = strdup(key);
    node->value = strdup(value);
    node->prev = NULL;
    node->next = NULL;
    return node;
}

static void free_node(Node *node){
    if(!node) return;
    free(node->key);
    free(node->value);
    free(node);
}

static void move_to_head(LRUCache *cache, Node *node) {
    if (cache->head == node) return;

    if(node->prev) node->prev->next = node->next;
    if(node->next) node->next->prev = node->prev;

    if(cache->tail == node){
        cache->tail = node->prev;
    }

    node->next = cache->head;
    node->prev = NULL;

    if (cache->head) {
        cache->head->prev = node;
    }
    cache->head = node;

    if(!cache->tail) {
        cache->tail = cache->head;
    }
}

static void remove_from_hash(LRUCache *cache, const char *key) {
    unsigned int index = hash_string(key, cache->hash_capacity);
    HashNode *current = cache->hash_table[index];
    HashNode *prev = NULL;

    while (current) {
        if (strcmp(current->cache_node->key, key) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                cache->hash_table[index] = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

static void insert_into_hash(LRUCache *cache, Node *node) {
    unsigned int index = hash_string(node->key, cache->hash_capacity);
    HashNode *hnode = (HashNode*)malloc(sizeof(HashNode));
    hnode->cache_node = node;
    hnode->next = cache->hash_table[index];
    cache->hash_table[index] = hnode;
}

static Node* find_in_hash(LRUCache *cache, const char *key){
    unsigned int index = hash_string(key, cache->hash_capacity);
    HashNode *current = cache->hash_table[index];

    while (current) {
        if (strcmp(current->cache_node->key, key) == 0) {
            return current->cache_node;
        }
        current = current->next;
    }
    return NULL;
}

LRUCache* lru_create(size_t capacity, size_t hash_capacity) {
    if(capacity == 0 || hash_capacity == 0) {
        return NULL;
    }

    LRUCache *cache = (LRUCache*)malloc(sizeof(LRUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->hash_capacity = hash_capacity;
    cache->head = NULL;
    cache->tail = NULL;
    cache->hash_table = (HashNode**)calloc(hash_capacity, sizeof(HashNode*));
    pthread_mutex_init(&cache->lock, NULL);
    
    return cache;
}

bool lru_put(LRUCache *cache, const char *key, const char *value) {
    if (!cache || !key || !value) return false;

    pthread_mutex_lock(&cache->lock);

    Node *node = find_in_hash(cache, key);
    if (node) {
        free(node->value);
        node->value = strdup(value);
        move_to_head(cache, node);
        pthread_mutex_unlock(&cache->lock);
        return true;
    }

    if (cache->size >= cache->capacity) {
        Node *tail = cache->tail;
        remove_from_hash(cache, tail->key);
        if (tail->prev) {
            tail->prev->next = NULL;
            cache->tail = tail->prev;
        } else {
            cache->head = NULL;
            cache->tail = NULL;
        }
        free_node(tail);
        cache->size--;
    }

    Node *new_node = create_node(key, value);
    insert_into_hash(cache, new_node);
    move_to_head(cache, new_node);
    cache->size++;

    pthread_mutex_unlock(&cache->lock);
    return true;
}

char* lru_get(LRUCache *cache, const char *key) {
    if (!cache || !key) return NULL;

    pthread_mutex_lock(&cache->lock);

    Node *node = find_in_hash(cache, key);
    if (!node) {
        pthread_mutex_unlock(&cache->lock);
        return NULL;
    }
    move_to_head(cache, node);
    char *value_copy = strdup(node->value);
    pthread_mutex_unlock(&cache->lock);
    return value_copy;
}

bool lru_remove(LRUCache *cache, const char *key) {
    if (!cache || !key) return false;

    pthread_mutex_lock(&cache->lock);

    Node *node = find_in_hash(cache, key);
    if (!node) {
        pthread_mutex_unlock(&cache->lock);
        return false;
    }

    remove_from_hash(cache, key);

    if (node->prev) {
        node->prev->next = node->next;
    } else {
        cache->head = node->next;
    }

    if (cache->head == node) {
        cache->head = node->next;
    } if (cache->tail == node) {
        cache->tail = node->prev;
    }

    free_node(node);
    cache->size--;

    pthread_mutex_unlock(&cache->lock);
    return true;
}

void lru_destroy(LRUCache *cache) {
    if (!cache) return;

    pthread_mutex_lock(&cache->lock);

    Node *curr = cache->head;
    while (curr) {
        Node *next = curr->next;
        free_node(curr);
        curr = next;
    }

    for (size_t i = 0; i < cache->hash_capacity; i++) {
        HashNode *hcurr = cache->hash_table[i];
        while (hcurr) {
            HashNode *hnext = hcurr->next;
            free(hcurr);
            hcurr = hnext;
        }
    }

    free(cache->hash_table);
    pthread_mutex_unlock(&cache->lock);
    pthread_mutex_destroy(&cache->lock);
    free(cache);
}

bool lru_save_to_disk(LRUCache *cache, const char *filepath) {
    if (!cache || !filepath) return false;

    pthread_mutex_lock(&cache->lock);
    FILE *file = fopen(filepath, "wb");
    if (!file) {
        pthread_mutex_unlock(&cache->lock);
        return false;
    }

    fwrite(&cache->size, sizeof(size_t), 1, file);

    Node *curr = cache->tail; // Write from LRU to MRU for easier restoring
    while (curr) {
        size_t klen = strlen(curr->key);
        size_t vlen = strlen(curr->value);

        fwrite(&klen, sizeof(size_t), 1, file);
        fwrite(curr->key, sizeof(char), klen, file);
        fwrite(&vlen, sizeof(size_t), 1, file);
        fwrite(curr->value, sizeof(char), vlen, file);

        curr = curr->prev;
    }

    fclose(file);
    pthread_mutex_unlock(&cache->lock);
    return true;
}

bool lru_load_from_disk(LRUCache *cache, const char *filepath) {
    if (!cache || !filepath) return false;

    FILE *file = fopen(filepath, "rb");
    if (!file) return false;

    size_t count = 0;
    if (fread(&count, sizeof(size_t), 1, file) != 1) {
        fclose(file);
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        size_t klen = 0, vlen = 0;
        if (fread(&klen, sizeof(size_t), 1, file) != 1) break;

        char *kbuf = (char*)malloc(klen + 1);
        if (fread(kbuf, sizeof(char), klen, file) != klen) {
            free(kbuf);
            break;
        }
        kbuf[klen] = '\0';

        if (fread(&vlen, sizeof(size_t), 1, file) != 1) {
            free(kbuf);
            break;
        }

        char *vbuf = (char*)malloc(vlen + 1);
        if (fread(vbuf, sizeof(char), vlen, file) != vlen) {
            free(kbuf);
            free(vbuf);
            break;
        }
        vbuf[vlen] = '\0';

        lru_put(cache, kbuf, vbuf);

        free(kbuf);
        free(vbuf);
    }

    fclose(file);
    return true;
}

void lru_print_cache(LRUCache *cache) {
    pthread_mutex_lock(&cache->lock);
    printf("Cache State [Size: %zu / %zu] (MRU -> LRU):\n", cache->size, cache->capacity);
    Node *curr = cache->head;
    while (curr) {
        printf("  [%s: %s]\n", curr->key, curr->value);
        curr = curr->next;
    }
    pthread_mutex_unlock(&cache->lock);
}