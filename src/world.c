#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <timing.h>

#include <cglm/cglm.h> 

#include <world.h>

world * world_init(gpu * gpu){
    world * w = malloc(sizeof(*w));
    assert(w);
    w->gpu = gpu;
    w->chunk_to_acquire = queue_init(2*TOTAL_CHUNKS*20);
    w->loaded_chunks = fixray_init(TOTAL_CHUNKS);
    // Enqueues all the starting chunks in a kind of spiral pattern (square radius increasing until render distance)
    for(int square_radius = 0; square_radius <= RENDER_DISTANCE; square_radius++){
    for (int z = -square_radius ; z < square_radius+1 ; z++){
        for (int x = -square_radius;  x < square_radius+1; x++){
            if (abs(z) < square_radius && abs(x) < square_radius ) continue;
            if (z == 0 && x == 0){
                chunk * c = chunk_init(x, z);
                uint64_t chunk_index = fixray_add(w->loaded_chunks, c);
                gpu_upload(w->gpu, chunk_index, c);
            }else{
                queue_enqueue(w->chunk_to_acquire, (void*)(intptr_t)x);
                queue_enqueue(w->chunk_to_acquire, (void*)(intptr_t)z);
            }
        }
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
    for (int z = -RENDER_DISTANCE; z < RENDER_DISTANCE; z++){
        for (int x = -RENDER_DISTANCE; x < RENDER_DISTANCE; x++){
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
            for (int i = old_x + RENDER_DISTANCE; i < new_x + RENDER_DISTANCE; i++) {
            for (int j = -RENDER_DISTANCE; j < RENDER_DISTANCE; j++) {
                array[global_index * 2] = i;
                array[global_index * 2 + 1] = new_z + j;
                global_index++;
                }
            }
            if (new_z > old_z) {
                for (int i = new_x - RENDER_DISTANCE; i < old_x + RENDER_DISTANCE; i++) {
                for (int j = old_z + RENDER_DISTANCE; j < new_z + RENDER_DISTANCE; j++) {
                    array[global_index * 2] = i;
                    array[global_index * 2 + 1] = j;
                    global_index++;
                    }
                }
            } else {
                for (int i = new_x - RENDER_DISTANCE; i < old_x + RENDER_DISTANCE; i++) {
                for (int j = new_z - RENDER_DISTANCE; j < old_z - RENDER_DISTANCE; j++) {
                    array[global_index * 2] = i;
                    array[global_index * 2 + 1] = j;
                    global_index++;
                    }
                }
            }
        } else {
            for (int i = new_x - RENDER_DISTANCE; i < old_x - RENDER_DISTANCE; i++) {
            for (int j = -RENDER_DISTANCE; j < RENDER_DISTANCE; j++) {
                array[global_index * 2] = i;
                array[global_index * 2 + 1] = new_z + j;
                global_index++;
                }
            }
            if (new_z > old_z) {
                for (int i = old_x - RENDER_DISTANCE; i < new_x + RENDER_DISTANCE; i++) {
                for (int j = old_z + RENDER_DISTANCE; j < new_z + RENDER_DISTANCE; j++) {
                    array[global_index * 2] = i;
                    array[global_index * 2 + 1] = j;
                    global_index++;
                    }
                }
            } else {
                for (int i = old_x - RENDER_DISTANCE; i < new_x + RENDER_DISTANCE; i++) {
                for (int j = new_z - RENDER_DISTANCE; j < old_z - RENDER_DISTANCE; j++) {
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
        uint64_t chunk_index = fixray_get_index(w->loaded_chunks, c);
        fixray_remove_from_index(w->loaded_chunks, chunk_index);

        gpu_unload(w->gpu, chunk_index);
        if (!chunk_in_cache(w, c)){
            world_append_chunk_cache(w, c);
        }

        index++;
    }
    // printf("Discarded %d chunks\n", index);
}


void world_update_acquired(world * w, int *acquired){
    int index = 0;
    int x,z;
    while(acquired[index*2] != INT_MAX){
        x = acquired[index*2 + 0];
        z = acquired[index*2 + 1];
        queue_enqueue(w->chunk_to_acquire, (void*)(intptr_t)x);
        queue_enqueue(w->chunk_to_acquire, (void*)(intptr_t)z);
        index++;
    }
    printf("Acquired %d\n", index);
}

bool chunk_in_range(chunk *c, int center_x, int center_z){
    return (c->x >= (center_x - (RENDER_DISTANCE)) &&
            c->z >= (center_z - (RENDER_DISTANCE)) &&
            c->x < (center_x + (RENDER_DISTANCE)) &&
            c->z < (center_z + (RENDER_DISTANCE)));
}

bool world_update_position(world * w, float x, float z){
    int new_center_x = ((int)x) / CHUNK_X_SIZE + (x<0.0 ? -1 : 0);
    int new_center_z = ((int)z) / CHUNK_Z_SIZE + (z<0.0 ? -1 : 0);
    size_t discard_index = 0;
    static chunk * discarded[TOTAL_CHUNKS+1]; // NULL terminated
    discarded[0] = NULL;
    static int acquired[TOTAL_CHUNKS*2+1]; //x0,y0,x1,y1 INT_MAX terminated;
    acquired[0] = INT_MAX;


    printf("center chunk faces %u, total sizeof in bytes : %zu\n", w->center_chunk->faces_count, chunk_sizeof(w->center_chunk));
    // get last chunk
    int count = 0;
    while (!queue_is_empty(w->chunk_to_acquire) && count < CHUNK_LOAD_PER_FRAME){
        int x = (int)(intptr_t)queue_dequeue(w->chunk_to_acquire);
        int z = (int)(intptr_t)queue_dequeue(w->chunk_to_acquire);
        if (x >= (new_center_x - (RENDER_DISTANCE)) &&
            z >= (new_center_z - (RENDER_DISTANCE)) &&
            x < (new_center_x + (RENDER_DISTANCE)) &&
            z < (new_center_z + (RENDER_DISTANCE))){
            chunk * c;
            if (chunk_in_cache_pos(w, x, z)){
                c = world_get_chunk_cache(w, x, z);
            }else{
                c = chunk_init(x, z);
            }
            uint64_t chunk_index = fixray_add(w->loaded_chunks, c);
            gpu_upload(w->gpu, chunk_index, c);
            count++;
        }
    }

    // ToDo : just like the acquired chunks this could be computed by a function instead of looping
    fixray_foreach(chunk *c, w->loaded_chunks){
        if (!chunk_in_range(c, new_center_x, new_center_z)){
            discarded[discard_index++] = c;
        }
    }
    discarded[discard_index] = NULL;
    world_update_discarded(w, discarded);
    // If we switched chunks
    if (new_center_x != w->center_chunk->x || new_center_z != w->center_chunk->z){
        printf("new center chunk %d,%d (previous %d,%d)\n", new_center_x, new_center_z, w->center_chunk->x, w->center_chunk->z);
        
        // Find the chunks that were discarded 
        // ToDo : just like the acquired chunks this could be computed by a function instead of looping
        
        world_compute_acquired_chunks(w->center_chunk->x, new_center_x, w->center_chunk->z, new_center_z, acquired);

        world_update_acquired(w, acquired);


        w->center_chunk = world_get_loaded_chunk(w, new_center_x, new_center_z);
        assert(w->center_chunk);
        printf("chunk cache bucket used %zu, total entries %zu\n", w->cache->used_buckets, w->cache->total_entries);
        return true;
    }
    return false;
}

chunk * world_get_loaded_chunk(world *w, int x, int z){
    fixray_foreach(chunk *c, w->loaded_chunks){
        if (c->x == x && c->z == z){
            return c;
        }
    }

    fprintf(stderr, "Tried to get a loaded chunk that wasn't loaded : %d,%d\n", x, z);
    return NULL;
}

// ask the gpu to upload the updated state of each chunk
void world_send_update(world *w){
    // Todo : add a stack to world and when a chunk is modified in world, in add the index to the stack, then on update empties stack and update each chunk to the gpu;
    fixray_foreach(chunk *c, w->loaded_chunks){
        if (c->faces_dirty || c->faces_dirty || c->rotations_dirty){
            uint64_t chunk_index = fixray_get_index(w->loaded_chunks, c);
            gpu_upload(w->gpu, chunk_index, c);
        }
    }
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
    fixray_foreach(chunk *c, w->loaded_chunks){
        chunk_cleanup(c);
    }

    htb_cleanup(w->cache, (void (*)(void*))chunk_cleanup);
    queue_cleanup(w->chunk_to_acquire);
    free(w);
}