#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <block.h>
#include <direction.h>
#include <atlas.h>

#define CHUNK_X_SIZE 16
#define CHUNK_Z_SIZE 16
#define CHUNK_Y_SIZE 128
#define CHUNK_LAYER_SIZE (CHUNK_X_SIZE * CHUNK_Z_SIZE)
#define CHUNK_SIZE (CHUNK_LAYER_SIZE * CHUNK_Y_SIZE)


// x,y,z , texStepX,texStepY
#define ATTRIBUTE_PER_VERTEX 5
#define VERTEX_PER_FACE 4
// Number of floats per face
#define FACE_FLOAT_COUNT (ATTRIBUTE_PER_VERTEX * VERTEX_PER_FACE)
// Number of bytes per face
#define FACE_BYTES (sizeof(float) * FACE_FLOAT_COUNT)

#define MAX_FACE_IN_CHUNK 10000

#define RENDER_DISTANCE 16*2
#define TOTAL_CHUNKS (RENDER_DISTANCE*RENDER_DISTANCE)

typedef struct chunk {
    int x; 
    int z;
    block blocks[CHUNK_SIZE];
    float * faces_offsets; // vec3
    unsigned int faces_count; // The number of visible block faces
    bool faces_dirty;

    float * textures_buffer;
    unsigned int textures_count;
    bool textures_dirty;
    float * rotations_values;
    unsigned int rotations_count;
    bool rotations_dirty;

    float view_time; // The time when the chunk was loaded/in view
} chunk;

chunk * chunk_init(int x, int y);

void chunk_add_block(chunk * c, block b, int index);
void chunk_remove_block(chunk * c, int index);

int chunk_pos_to_index(vec3 pos);
float chunk_norm_pos_x(chunk *c, float x);
float chunk_norm_pos_z(chunk *c, float z);
bool chunk_is_pos_inside_block(chunk const * c, vec3 pos);

// float * chunk_generate_static_mesh(unsigned int *vertice_count);
// unsigned int * chunk_get_elements(chunk * c, unsigned int *vertex_count);
// void chunk_generate_elements_buffer(chunk * c);

uint64_t chunk_get_id(chunk * c);
uint64_t chunk_get_id_pos(int x, int z);

bool chunk_is_solid_direction(chunk const * c, int block_index, direction d);

float * chunk_get_faces_offsets(chunk * c, unsigned int *instance_count);
float * chunk_get_textures(chunk * c, unsigned int * faces_count, atlas * a);
float * chunk_get_rotations_values(chunk * c, unsigned int *instance_count);

int chunk_block_x(int block_index);
int chunk_block_y(int block_index);
int chunk_block_z(int block_index);


void chunk_cleanup(chunk * c);