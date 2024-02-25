#pragma once

#include <stdlib.h>
#include <chunk.h>
#include <world.h>
#include <atlas.h>

#define DEBUG_GL(command) do { \
    command; \
    GLenum error = glGetError(); \
    if (error != GL_NO_ERROR) { \
        fprintf(stderr, "OpenGL Error at %s:%d - Code %d\n", __FILE__, __LINE__, error); \
    } \
} while(0)


typedef unsigned int uint;
typedef struct gpu {
    atlas * atlas;
    size_t stack_index[TOTAL_CHUNKS];
    size_t stack_top;
    void* chunk_index[TOTAL_CHUNKS]; 

    // OpenGL Objects
    uint VAO[TOTAL_CHUNKS];
    uint VBO_face;
    uint IBA;   // Instance Buffer Attributes
} gpu;

gpu * gpu_init();

void gpu_set_VAO(gpu *g, size_t vao_index);

void gpu_unload(gpu* g, size_t chunk_index);

void gpu_update(gpu* g, size_t chunk_index);
// void gpu_update(gpu* g, chunk *c);

size_t gpu_upload(gpu* g, chunk *c);
// void gpu_upload(gpu* g, chunk *c);

size_t gpu_get_index(gpu* g, chunk *c);

void gpu_cleanup(gpu* g);