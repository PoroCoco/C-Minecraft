#pragma once

#include <stdbool.h>

#include <chunk.h>

#define RENDER_DISTANCE 8
#define TOTAL_CHUNKS (RENDER_DISTANCE*RENDER_DISTANCE)


typedef struct world {
    // Needs a set somewhere to quickly check if a chunk is cached (retrieve it) or have been generated (load it) or needs to be generated (gen it)

    // Make this a dynamic array 
    chunk ** cached_chunks;
    unsigned int cached_chunks_count;
    chunk * loaded_chunks[TOTAL_CHUNKS];
    chunk * center_chunk; // chunk in the center of the loaded world, where the player currently is

} world;


world * world_init();

bool world_update_position(world * w, float x, float z);
void world_load_chunk(world * w, int x, int z);
void world_unload_chunk(world * w, int x, int z);
chunk * world_get_loaded_chunk(world *w, int x, int z);
chunk * world_get_chunk_direction(world *w, chunk const * c, direction d);

void world_cleanup(world * w);