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


// x,y,z
#define ATTRIBUTE_PER_VERTEX 3
#define VERTEX_PER_FACE 4
// Number of floats per face
#define FACE_FLOAT_COUNT (ATTRIBUTE_PER_VERTEX * VERTEX_PER_FACE)
// Number of bytes per face
#define FACE_BYTES (sizeof(float) * FACE_FLOAT_COUNT)

#define MAX_FACE_IN_CHUNK 25000

#define RENDER_DISTANCE 32
#define WORLD_CHUNKS_LENGTH (RENDER_DISTANCE*2)
#define TOTAL_CHUNKS (WORLD_CHUNKS_LENGTH * WORLD_CHUNKS_LENGTH)

#define PACKED_4BYTES_COUNT 2
/*
Instance data packing:

Data : |OffsetX|OffsetY |OffsetZ|TextureId|RotationIndex|EMPTY|ScaleX|ScaleY|
Bits : |0-----5|6-----12|13---18|19-----26|27---------29|30-31|0----6|7---13|
Size : |   6   |    7   |   6   |    8    |      3      |  2  |   7  |   7  |
Float: |------------------------INT1--------------------------|-------------------INT2-------------------|

Given the chunk sizes, the offsets x,z ranges (0-63) and y (0-127), therefore packed on 6 and 7 bits respectively
The atlas contains up to 256 textures so the textureId is packed on 8 bits
A cube face can only be rotated in 6 directions --> 3 bits
////The greedy meshing scales should be 0-63 for X and 0-127 for Y. This could be dangerous as top and bottom faces switch X,Y scales but right now they are only limited to a 64*64 surface. So 6 bits for X, 7 for Y;
Actually just put the 2 of them on 7 bits for now, I'll reduce it later (surely i won't forget that :) )

All of the instance data can be reduced from 7 floats down to 2 float with some remaining futur space. If space becomes really sparse, reordering can be done to use the 2 empty endings bits of float1
    // value &= 0;
    // printBits(value);
    
    // value |= (uint8_t)x;
    // printBits(value);
    // value |= ((uint8_t)z << 6);
    // printBits(value);
    // value |= ((uint8_t)y << 12);
    // printBits(value);
    // value |= ((uint8_t)textureId << 19);
    // printBits(value);
    // value |= ((uint8_t)rota << 27);
    // printBits(value);

    // // Unpacking the coords
    // int unpack_x = ((value) & 0x3F);
    // int unpack_z = ((value >> 6) & 0x3F);
    // int unpack_y = ((value >> 12) & 0x7F);
    // int unpack_texture = ((value >> 19) & 0xFF);
    // int unpack_rota = ((value >> 27) & 0x7);
*/

typedef struct chunk {
    int x; 
    int z;
    uint8_t block_ids[CHUNK_SIZE];
    
    int32_t faces_count; // The number of visible block faces
    uint32_t * faces_packed_data;
    bool mesh_dirty;
    bool in_frustum;

    float timestap_generation; // The time when the chunk was generated
    bool ready;
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

void chunk_hotreload_genetor(bool reload);

void chunk_cleanup(chunk * c);