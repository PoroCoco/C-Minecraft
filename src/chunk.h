#pragma once

#include <stdbool.h>

#include <block.h>
#include <direction.h>
#include <atlas.h>

#define CHUNK_X_SIZE 16
#define CHUNK_Z_SIZE 16
#define CHUNK_Y_SIZE 128
#define CHUNK_LAYER_SIZE (CHUNK_X_SIZE * CHUNK_Z_SIZE)
#define CHUNK_SIZE (CHUNK_LAYER_SIZE * CHUNK_Y_SIZE)


// x,y,z , textCoordX,textCoordY
#define ATTRIBUTE_PER_VERTEX 5 
#define VERTEX_PER_FACE 6 
// Number of floats per face
#define FACE_FLOAT_COUNT (ATTRIBUTE_PER_VERTEX * VERTEX_PER_FACE)
// Number of bytes per face
#define FACE_BYTES (sizeof(float) * FACE_FLOAT_COUNT)

typedef struct chunk {
    int x; 
    int z;
    block blocks[CHUNK_SIZE];
    unsigned int *elements; // The openGL element buffer data
    unsigned int elements_count;
    bool elements_dirty;
} chunk;

chunk * chunk_init(int x, int y);

void chunk_add_block(chunk * c, block b, int index);
void chunk_remove_block(chunk * c, int index);

int chunk_pos_to_index(vec3 pos);
float chunk_norm_pos_x(chunk *c, float x);
float chunk_norm_pos_z(chunk *c, float z);
bool chunk_is_pos_inside_block(chunk const * c, vec3 pos);

float * chunk_generate_static_mesh(atlas * a, int *vertice_count);
unsigned int * chunk_get_elements(chunk * c, int *vertex_count, atlas * a);
void chunk_generate_elements_buffer(chunk * c, atlas * a);

void chunk_cleanup(chunk * c);