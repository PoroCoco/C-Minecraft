#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <block.h>
#include <direction.h>
#include <atlas.h>

#define CHUNK_X_SIZE 64
#define CHUNK_Z_SIZE 64
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

#define MAX_FACE_IN_CHUNK 100000

#define RENDER_DISTANCE 4*2
#define TOTAL_CHUNKS (RENDER_DISTANCE*RENDER_DISTANCE)

typedef struct chunk {
    int x; 
    int z;
    uint8_t block_ids[CHUNK_SIZE];
    
    unsigned int faces_count; // The number of visible block faces
    float * faces_offsets; // vec3
    float * faces_textures;
    float * faces_rotations;
    bool faces_dirty;
    bool textures_dirty;
    bool rotations_dirty;
    bool in_frustum;

    float timestap_generation; // The time when the chunk was generated
} chunk;

chunk * chunk_init(int x, int y);

void chunk_add_block(chunk * c, uint8_t block_id, int index);
void chunk_remove_block(chunk * c, int index);

float chunk_norm_pos_x(chunk *c, float x);
float chunk_norm_pos_z(chunk *c, float z);
bool chunk_is_pos_inside_block(chunk const * c, vec3 pos);

// Creates an id for the given chunk from its x,z
uint64_t chunk_get_id(chunk * c);
// Same as chunk_get_id but with raw coord
uint64_t chunk_get_id_pos(int x, int z);

int chunk_pos_to_index(vec3 pos);
int chunk_block_x(int block_index);
int chunk_block_y(int block_index);
int chunk_block_z(int block_index);

size_t chunk_sizeof(chunk * c);

void chunk_cleanup(chunk * c);