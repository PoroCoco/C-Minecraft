#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <chunk.h>
#include <atlas.h>

#define DEBUG_GL(command) do { \
    command; \
    GLenum error = glGetError(); \
    if (error != GL_NO_ERROR) { \
        fprintf(stderr, "OpenGL Error at %s:%d - Code %d\n", __FILE__, __LINE__, error); \
    } \
} while(0)


typedef struct gpu {
    atlas * atlas;

    // OpenGL Objects
    uint32_t VAO[TOTAL_CHUNKS];
    uint32_t VBO_face;
    uint32_t IBA[TOTAL_CHUNKS];   // Instance Buffer Attributes
    uint32_t instances_count[TOTAL_CHUNKS];
} gpu;

gpu * gpu_init(atlas* atlas);

void gpu_set_VAO(gpu*gpu, uint64_t vao_index);

void gpu_unload(gpu* gpu, uint64_t chunk_index);

void gpu_upload(gpu* gpu, uint64_t chunk_index, chunk* c);

void gpu_draw(gpu* gpu, uint64_t index);

void gpu_cleanup(gpu* gpu);