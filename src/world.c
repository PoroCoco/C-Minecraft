#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <world.h>

world * world_init(){
    world * w = malloc(sizeof(*w));
    assert(w);
    w->center_chunk = (point){.x = 0, .z = 0};
    printf("world init center at %d,%d\n", w->center_chunk.x, w->center_chunk.z);
    int index = 0;
    for (int z = -RENDER_DISTANCE/2 ; z < RENDER_DISTANCE/2 ; z++){
        for (int x = -RENDER_DISTANCE/2;  x < RENDER_DISTANCE/2; x++){
            w->loaded_chunks[index] = chunk_init(x, z);
            index++;
        }
    }

    w->cached_chunks_count = 0;
    w->cached_chunks = NULL;
    
    return w;
}


bool chunk_in_cache(world * w, int x , int z){
    for (size_t i = 0; i < w->cached_chunks_count; i++){
        if (w->cached_chunks[i]->x == x && w->cached_chunks[i]->z == z){
            return true;
        }
    }
    return false;
}

void world_append_chunk_cache(world * w, chunk * chunk){
    w->cached_chunks_count++;
    w->cached_chunks = realloc(w->cached_chunks, sizeof(*w->cached_chunks) * w->cached_chunks_count);
    assert(w->cached_chunks);
    w->cached_chunks[w->cached_chunks_count - 1] = chunk;
}

chunk * world_get_chunk_cache(world * w, int x , int z){
    for (size_t i = 0; i < w->cached_chunks_count; i++){
        if (w->cached_chunks[i]->x == x && w->cached_chunks[i]->z == z){
            return w->cached_chunks[i];
        }
    }
    fprintf(stderr, "Tried to get a chunk from the cache that wasn't inside !");
    return NULL;
}

bool world_update_position(world * w, float x, float z){
    point new_center = {.x = ((int)x) /CHUNK_X_SIZE, .z = ((int)z) / CHUNK_Z_SIZE};

    if (new_center.x != w->center_chunk.x || new_center.z != w->center_chunk.z){
        printf("new center chunk %d,%d (previous %d,%d)\n", new_center.x, new_center.z, w->center_chunk.x, w->center_chunk.z);
        w->center_chunk = new_center;
        int index = 0;
        chunk * chunk;
        for (int z = -RENDER_DISTANCE/2 ; z < RENDER_DISTANCE/2 ; z++){
            for (int x = -RENDER_DISTANCE/2;  x < RENDER_DISTANCE/2; x++){
                chunk = w->loaded_chunks[index];
                // Put old chunk in the cache
                if (!chunk_in_cache(w, chunk->x, chunk->z)){
                    world_append_chunk_cache(w, chunk);
                }

                // Get new chunk from cache or generate it
                if (chunk_in_cache(w, x + w->center_chunk.x, z + w->center_chunk.z)){
                    w->loaded_chunks[index] = world_get_chunk_cache(w, x + w->center_chunk.x, z + w->center_chunk.z);
                    assert(w->loaded_chunks[index]);
                }else{
                    w->loaded_chunks[index] = chunk_init(x + w->center_chunk.x, z + w->center_chunk.z);
                }
                index++;
            }
        }
        return true;
    }
    return false;
}


void world_load_chunk(world * w, int x, int z);
void world_unload_chunk(world * w, int x, int z);

void world_cleanup(world * w){
    for (size_t i = 0; i < TOTAL_CHUNKS; i++){
        chunk_cleanup(w->loaded_chunks[i]);
    }
    for (size_t i = 0; i < w->cached_chunks_count; i++){
        chunk_cleanup(w->cached_chunks[i]);
    }
    free(w->cached_chunks);

    free(w);
}