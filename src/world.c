#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <cglm/cglm.h> 

#include <world.h>

world * world_init(){
    world * w = malloc(sizeof(*w));
    assert(w);
    int index = 0;
    for (int z = -RENDER_DISTANCE/2 ; z < RENDER_DISTANCE/2 ; z++){
        for (int x = -RENDER_DISTANCE/2;  x < RENDER_DISTANCE/2; x++){
            w->loaded_chunks[index] = chunk_init(x, z);
            index++;
        }
    }

    w->center_chunk = world_get_loaded_chunk(w, 0, 0);
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
    int new_center_x = ((int)x) / CHUNK_X_SIZE + (x<0.0 ? -1 : 0);
    int new_center_z = ((int)z) / CHUNK_Z_SIZE + (z<0.0 ? -1 : 0);

    if (new_center_x != w->center_chunk->x || new_center_z != w->center_chunk->z){
        printf("new center chunk %d,%d (previous %d,%d)\n", new_center_x, new_center_z, w->center_chunk->x, w->center_chunk->z);
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
                if (chunk_in_cache(w, x + new_center_x, z + new_center_z)){
                    w->loaded_chunks[index] = world_get_chunk_cache(w, x + new_center_x, z + new_center_z);
                    assert(w->loaded_chunks[index]);
                }else{
                    w->loaded_chunks[index] = chunk_init(x + new_center_x, z + new_center_z);
                }
                index++;
            }
        }
        w->center_chunk = world_get_loaded_chunk(w, new_center_x, new_center_z);
        assert(w->center_chunk);
        return true;
    }
    return false;
}

chunk * world_get_loaded_chunk(world *w, int x, int z){
    for (size_t i = 0; i < TOTAL_CHUNKS; i++){
        if (w->loaded_chunks[i]->x == x && w->loaded_chunks[i]->z == z){
            return w->loaded_chunks[i];
        }
    }

    fprintf(stderr, "Tried to get a loaded chunk that wasn't loaded : %d,%d\n", x, z);
    return NULL;
}

chunk * world_get_chunk_direction(world *w, chunk const * c, direction d){
    switch (d)
    {
    case NORTH:
        return world_get_loaded_chunk(w, c->x, c->z - 1);
    case SOUTH:
        return world_get_loaded_chunk(w, c->x, c->z + 1);
    case WEST:
        return world_get_loaded_chunk(w, c->x - 1, c->z);
    case EAST:
        return world_get_loaded_chunk(w, c->x + 1, c->z);
    default:
        fprintf(stderr, "Tried to get a chunk in the TOP or BOTTOM direction !\n");
        return (chunk *)c;
    }
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