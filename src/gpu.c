#include <assert.h>
#include <gpu.h>


gpu * gpu_init(){
    gpu *g = malloc(sizeof(*g));

    g->stack_top = TOTAL_CHUNKS-1;
    for (size_t i = 0; i < TOTAL_CHUNKS; i++){
        g->stack_index[i] = TOTAL_CHUNKS-i-1;
        g->chunk_index[i] = 0;
    }
    
    // OpenGL Objects




    return g;
}

void gpu_unload(gpu* g, size_t chunk_index){
    assert(chunk_index < TOTAL_CHUNKS);
    g->stack_index[g->stack_top++] = chunk_index;
    assert(g->stack_top < TOTAL_CHUNKS);
}

// size_t gpu_upload(gpu* g, chunk *c){
//     size_t free_index = g->stack_index[g->stack_top--];
//     g->chunk_index[free_index] = (void*)c;

//     gpu_update(g, c);
//     return free_index;
// }

// void gpu_update(gpu* g, size_t chunk_index){
//     chunk *c = g->chunk_index[chunk_index];
//     unsigned int instances_count = 0;
//         // DEBUG_GL(glBindVertexArray(VAO[chunk_index]));
//         // DEBUG_GL(glBindBuffer(GL_ARRAY_BUFFER, IBA[chunk_index]));
        
//         // // Updating the IBA 
//         // float * instaces_offsets = chunk_get_faces_offsets(c, &instances_count);
//         // assert(MAX_FACE_IN_CHUNK > instances_count[chunk_index]); // Need to increase the max face per chunk
//         // DEBUG_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*instances_count[chunk_index], instaces_offsets));

//         // float * textures_start = chunk_get_textures(c, &instances_count, g->atlas);
//         // assert(MAX_FACE_IN_CHUNK > instances_count[chunk_index]); // Need to increase the max face per chunk
//         // DEBUG_GL(glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*MAX_FACE_IN_CHUNK*3, sizeof(float)*2*instances_count[chunk_index], textures_start));

//         // float * rotations_values = chunk_get_rotations_values(c, &instances_count);
//         // assert(MAX_FACE_IN_CHUNK > instances_count[chunk_index]); // Need to increase the max face per chunk
//         // DEBUG_GL(glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*MAX_FACE_IN_CHUNK*5, sizeof(float)*1*instances_count[chunk_index], rotations_values));
// }

size_t gpu_get_index(gpu* g, chunk *c){
    for (size_t i = 0; i < TOTAL_CHUNKS; i++){
        if (g->chunk_index[i] == (void*)c){
            return i;
        }
    }
    fprintf(stderr, "Couldn't find the index for the given chunk on the gpu\n");
    return -1;
}

void gpu_cleanup(gpu* g){
    free(g);
}