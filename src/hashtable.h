#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <pthread.h>
#endif

typedef struct bucket{
    void ** values;
    int64_t* keys;
    uint64_t count;
    uint64_t size;
} bucket;

typedef struct hashtable{
    bucket* table;
    uint64_t size;
    uint64_t used_buckets;
    uint64_t total_entries;

    #ifdef _WIN32
        HANDLE mutex;
    #elif __linux__
        pthread_mutex_t mutex;
    #endif
} htb;


htb * htb_init(uint64_t size);
void htb_add(htb* h, int64_t key, void* value);
void htb_remove(htb* h, int64_t key);
bool htb_exist(htb* h, int64_t key);
void * htb_get(htb* h, int64_t key);
void htb_cleanup(htb* h, void free_value (void*));