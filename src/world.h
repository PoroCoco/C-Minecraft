#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <chunk.h>
#include <gpu.h>
#include <hashtable.h>
#include <fixed_array.h>
#include <queue_og.h>

#define CHUNK_LOAD_PER_FRAME 8

typedef struct world {
    gpu* gpu;
    htb* cache;
    fixray* loaded_chunks;
    chunk* center_chunk; // chunk in the center of the loaded world, where the player currently is
    queue_og* chunk_to_acquire;
} world;


world * world_init(gpu * gpu);

bool world_update_position(world* w, float x, float z);
void world_load_chunk(world* w, int x, int z);
void world_unload_chunk(world* w, int x, int z);
chunk* world_get_loaded_chunk(world* w, int x, int z);
chunk* world_get_chunk_direction(world* w, chunk const* c, direction d);
void world_send_update(world* w);
void * thread_generate_chunk(void * args);

void world_cleanup(world* w);