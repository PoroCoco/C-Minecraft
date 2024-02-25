#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>

#include <cglm/cglm.h> 

#include <world.h>

world * world_init(){
    world * w = malloc(sizeof(*w));
    assert(w);
    size_t index = 0;
    for (int z = -RENDER_DISTANCE/2 ; z < RENDER_DISTANCE/2 ; z++){
        for (int x = -RENDER_DISTANCE/2;  x < RENDER_DISTANCE/2; x++){
            w->loaded_chunks[index] = chunk_init(x, z);
            index++;
        }
    }

    w->center_chunk = world_get_loaded_chunk(w, 0, 0);
    w->cache = htb_init(100000);
    
    return w;
}

bool chunk_in_cache_pos(world * w, int x, int z){
    return htb_exist(w->cache, chunk_get_id_pos(x, z));
}
bool chunk_in_cache(world * w, chunk *c){
    return htb_exist(w->cache, chunk_get_id(c));
}

void world_append_chunk_cache(world * w, chunk * chunk){
    htb_add(w->cache, chunk_get_id(chunk), chunk);
}

chunk * world_get_chunk_cache(world * w, int x , int z){
    chunk *c = htb_get(w->cache, chunk_get_id_pos(x, z));
    if (c == NULL){
        fprintf(stderr, "Tried to get a chunk from the cache that wasn't inside !");
    }
    return c;
}


/*

on chunk moving -> find chunks discarded/acquired -> 

*/

void world_compute_chunk_in_range(int center_x ,int center_z, int * array){
    int index = 0;
    for (int z = -RENDER_DISTANCE/2; z < RENDER_DISTANCE/2; z++){
        for (int x = -RENDER_DISTANCE/2; x < RENDER_DISTANCE/2; x++){
            array[index*2 +0] = x + center_x;
            array[index*2 +1] = z + center_z;
            index++;
        }
    }
    array[index*2] = INT_MAX;
}

void world_compute_acquired_chunks(int old_x, int new_x, int old_z, int new_z, int *array) {
    int global_index = 0;

    if (abs(new_x-old_x) < RENDER_DISTANCE && abs(new_z-old_z) < RENDER_DISTANCE) {
        if (new_x > old_x) {
            for (int i = old_x + RENDER_DISTANCE / 2; i < new_x + RENDER_DISTANCE / 2; i++) {
            for (int j = -RENDER_DISTANCE / 2; j < RENDER_DISTANCE / 2; j++) {
                array[global_index * 2] = i;
                array[global_index * 2 + 1] = new_z + j;
                global_index++;
                }
            }
            if (new_z > old_z) {
                for (int i = new_x - RENDER_DISTANCE / 2; i < old_x + RENDER_DISTANCE / 2; i++) {
                for (int j = old_z + RENDER_DISTANCE / 2; j < new_z + RENDER_DISTANCE / 2; j++) {
                    array[global_index * 2] = i;
                    array[global_index * 2 + 1] = j;
                    global_index++;
                    }
                }
            } else {
                for (int i = new_x - RENDER_DISTANCE / 2; i < old_x + RENDER_DISTANCE / 2; i++) {
                for (int j = new_z - RENDER_DISTANCE / 2; j < old_z - RENDER_DISTANCE / 2; j++) {
                    array[global_index * 2] = i;
                    array[global_index * 2 + 1] = j;
                    global_index++;
                    }
                }
            }
        } else {
            for (int i = new_x - RENDER_DISTANCE / 2; i < old_x - RENDER_DISTANCE / 2; i++) {
            for (int j = -RENDER_DISTANCE / 2; j < RENDER_DISTANCE / 2; j++) {
                array[global_index * 2] = i;
                array[global_index * 2 + 1] = new_z + j;
                global_index++;
                }
            }
            if (new_z > old_z) {
                for (int i = old_x - RENDER_DISTANCE / 2; i < new_x + RENDER_DISTANCE / 2; i++) {
                for (int j = old_z + RENDER_DISTANCE / 2; j < new_z + RENDER_DISTANCE / 2; j++) {
                    array[global_index * 2] = i;
                    array[global_index * 2 + 1] = j;
                    global_index++;
                    }
                }
            } else {
                for (int i = old_x - RENDER_DISTANCE / 2; i < new_x + RENDER_DISTANCE / 2; i++) {
                for (int j = new_z - RENDER_DISTANCE / 2; j < old_z - RENDER_DISTANCE / 2; j++) {
                    array[global_index * 2] = i;
                    array[global_index * 2 + 1] = j;
                    global_index++;
                    }
                }
            }
        }
        array[global_index * 2] = INT_MAX;
    }else{
        world_compute_chunk_in_range(new_x, new_z, array);
    }
}



void world_update_discarded(world * w, chunk ** discarded){
    int index = 0;
    while(discarded[index] != NULL){
        chunk *c = discarded[index];
        if (!chunk_in_cache(w, c)){
            world_append_chunk_cache(w, c);
        }
        // Remove from gpu

        index++;
    }
}


void world_acquire_chunk(){

    return ;
}
void world_update_acquired(world * w, int *acquired){
    int index = 0;
    int x,z;
    while(acquired[index*2] != INT_MAX){
        // create thread that create the chunk
        x = acquired[index*2 + 0];
        z = acquired[index*2 + 1];
        if (chunk_in_cache_pos(w, x, z)){
            w->loaded_chunks[index] = world_get_chunk_cache(w, x, z);
            assert(w->loaded_chunks[index]);
        }else{
            w->loaded_chunks[index] = chunk_init(x, z);
        }
        // Add to gpu

        index++;
    }
}

bool chunk_in_range(chunk *c, int center_x, int center_z){
    return (c->x >= (center_x - (RENDER_DISTANCE/2)) &&
            c->z >= (center_z - (RENDER_DISTANCE/2)) &&
            c->x < (center_x + (RENDER_DISTANCE/2)) &&
            c->z < (center_z + (RENDER_DISTANCE/2))
           );
}

bool world_update_position(world * w, float x, float z){
    int new_center_x = ((int)x) / CHUNK_X_SIZE + (x<0.0 ? -1 : 0);
    int new_center_z = ((int)z) / CHUNK_Z_SIZE + (z<0.0 ? -1 : 0);
    size_t discard_index = 0;
    static chunk * discarded[TOTAL_CHUNKS+1];
    static int acquired[TOTAL_CHUNKS*2+1]; //x0,y0,x1,y1

    // If we switched chunks
    if (new_center_x != w->center_chunk->x || new_center_z != w->center_chunk->z){
        printf("new center chunk %d,%d (previous %d,%d)\n", new_center_x, new_center_z, w->center_chunk->x, w->center_chunk->z);
        int index = 0;
        for (size_t i = 0; i < TOTAL_CHUNKS; i++){
            // if (chunk_in_range(w->loaded_chunks[i], new_center_x, new_center_z)){
                // continue;
            // }else{
                discarded[discard_index++] = w->loaded_chunks[i];
            // }
        }
        discarded[discard_index] = NULL;
        world_update_discarded(w, discarded);

        // world_compute_acquired_chunks(w->center_chunk->x, w->center_chunk->z, new_center_x, new_center_z, acquired);
        world_compute_chunk_in_range(new_center_x, new_center_z, acquired);
        world_update_acquired(w, acquired);


        // chunk * chunk;
        // for (int z = -RENDER_DISTANCE/2 ; z < RENDER_DISTANCE/2 ; z++){
        //     for (int x = -RENDER_DISTANCE/2;  x < RENDER_DISTANCE/2; x++){
        //         chunk = w->loaded_chunks[index];
        //         // Put old chunk in the cache
        //         if (!chunk_in_cache(w, chunk->x, chunk->z)){
        //             world_append_chunk_cache(w, chunk);
        //         }

        //         // Get new chunk from cache or generate it
        //         if (chunk_in_cache(w, x + new_center_x, z + new_center_z)){
        //             w->loaded_chunks[index] = world_get_chunk_cache(w, x + new_center_x, z + new_center_z);
        //             assert(w->loaded_chunks[index]);
        //         }else{
        //             w->loaded_chunks[index] = chunk_init(x + new_center_x, z + new_center_z);
        //         }
        //         index++;
        //     }
        // }
        w->center_chunk = world_get_loaded_chunk(w, new_center_x, new_center_z);
        assert(w->center_chunk);
        printf("chunk cache bucket used %zu, total entries %zu\n", w->cache->used_buckets, w->cache->total_entries);
        return true;
    }
    return false;
}

chunk * world_get_loaded_chunk(world *w, int x, int z){
    for (size_t i = 0; i < TOTAL_CHUNKS; i++){
        if (w->loaded_chunks[i] != NULL && w->loaded_chunks[i]->x == x && w->loaded_chunks[i]->z == z){
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
    htb_cleanup(w->cache, (void (*)(void*))chunk_cleanup);

    free(w);
}