#pragma once

#include <stdbool.h>

#include <block.h>

#define CHUNK_X_SIZE 16
#define CHUNK_Z_SIZE 16
#define CHUNK_Y_SIZE 128
#define CHUNK_LAYER_SIZE (CHUNK_X_SIZE * CHUNK_Z_SIZE)
#define CHUNK_SIZE (CHUNK_LAYER_SIZE * CHUNK_Y_SIZE)


typedef enum direction {
    DIR_START = 0,
    TOP = 0,
    BOTTOM,
    NORTH,
    SOUTH,
    EAST,
    WEST,
    DIR_COUNT
} direction;

typedef struct chunk {
    int x; 
    int y;
    block blocks[CHUNK_SIZE];
} chunk;

chunk * chunk_init(int x, int y);

float * chunk_generate_vertices(chunk * c, int *vertex_count);

void chunk_cleanup(chunk * c);