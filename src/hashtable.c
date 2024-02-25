#include <hashtable.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

void _htb_get_lock(htb* h){
    #ifdef _WIN32
        WaitForSingleObject(h->mutex, INFINITE);
    #elif __linux__
        pthread_mutex_lock(&h->mutex);
    #endif
}

void _htb_release_lock(htb* h){
    #ifdef _WIN32
        ReleaseMutex(h->mutex);
    #elif __linux__
        pthread_mutex_unlock(&h->mutex);
    #endif
}


uint64_t hash(htb* h, int64_t key){
    key = ((key >> 32) ^ key) * 0x45d9f3b;
    key = ((key >> 32) ^ key) * 0x45d9f3b;
    key = (key >> 32) ^ key;
    return key%h->size;
}

htb * htb_init(uint64_t size){
    htb * h = malloc(sizeof(*h));
    h->size = size;
    h->used_buckets = 0;
    h->total_entries = 0;
    h->table = malloc(sizeof(*h->table) * size);
    for (size_t i = 0; i < h->size; i++){
        h->table[i].size = 1;
        h->table[i].values = malloc(sizeof(h->table[i].values));
        h->table[i].keys = malloc(sizeof(h->table[i].keys));
        h->table[i].count = 0;
    }
    #ifdef _WIN32
        h->mutex = CreateMutex(NULL, FALSE,NULL);
    #elif __linux__
        pthread_mutex_init(&h->mutex, NULL);
    #endif
    return h;
}

void htb_add(htb* h, int64_t key, void* value){
    bucket *b = h->table + hash(h, key);
    _htb_get_lock(h);
    if (b->count == 0){
        h->used_buckets++;
    }
    if (b->count == b->size){
        b->size *= 2;
        b->keys = realloc(b->keys, b->size * sizeof(*(b->keys)));
        b->values = realloc(b->values, b->size * sizeof(*(b->values)));
    }
    b->values[b->count] = value;
    b->keys[b->count] = key;
    b->count++;
    h->total_entries++;
    _htb_release_lock(h);
}

void htb_remove(htb* h, int64_t key){
    bucket *b = h->table + hash(h, key);
    _htb_get_lock(h);
    for (size_t i = 0; i < b->count; i++){
        if (b->keys[i] == key){
            // Bucket doesn't scale down
            for (size_t j = i; j < b->count-1; j++){
                b->keys[j] = b->keys[j+1]; 
                b->values[j] = b->values[j+1]; 
            }
            b->count--;
            h->total_entries--;
            if (b->count == 0){
                h->used_buckets--;
            }
            _htb_release_lock(h);
            return;
        }
    }
    fprintf(stderr, "Tried to remove a key that wasn't stored !\n");
    _htb_release_lock(h);
}

bool htb_exist(htb* h, int64_t key){
    bucket *b = h->table + hash(h, key);
    _htb_get_lock(h);
    for (size_t i = 0; i < b->count; i++){
        if (b->keys[i] == key){
            _htb_release_lock(h);
            return true;
        }
    }
    _htb_release_lock(h);
    return false;
}

void * htb_get(htb* h, int64_t key){
    bucket *b = h->table + hash(h, key);
    _htb_get_lock(h);
    for (size_t i = 0; i < b->count; i++){
        if (b->keys[i] == key){
            _htb_release_lock(h);
            return b->values[i];
        }
    }
    fprintf(stderr, "Tried to get a key that wasn't stored !\n");
    _htb_release_lock(h);
    return NULL;
}
void htb_cleanup(htb* h, void free_value (void*)){
    for (size_t i = 0; i < h->size; i++){
        bucket *b = h->table + i;
        for (size_t i = 0; i < b->count; i++){
            free_value(b->values[i]);
        }
        free(b->values);
        free(b->keys);
    }
    free(h->table);
    #ifdef _WIN32
        CloseHandle(h->mutex);
    #elif __linux__
        pthread_mutex_destroy(&h->mutex);
    #endif
    free(h);
}