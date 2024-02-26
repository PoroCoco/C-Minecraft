#pragma once

#include <stdbool.h>
#include <stdint.h>

#define RENDER_DISTANCE 16*2
#define TOTAL_CHUNKS (RENDER_DISTANCE*RENDER_DISTANCE)

#include <chunk.h>
#include <gpu.h>
#include <hashtable.h>
#include <fixed_array.h>


typedef struct world {
    htb *cache;
    htb *loaded_chunks_index;
    fixray * loaded_chunks;
    chunk * center_chunk; // chunk in the center of the loaded world, where the player currently is
} world;


world * world_init();

bool world_update_position(world * w, float x, float z);
void world_load_chunk(world * w, int x, int z);
void world_unload_chunk(world * w, int x, int z);
chunk * world_get_loaded_chunk(world *w, int x, int z);
chunk * world_get_chunk_direction(world *w, chunk const * c, direction d);

void world_cleanup(world * w);