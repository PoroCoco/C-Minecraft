#pragma once

#include <chunk.h>

bool chunk_is_solid_direction(chunk const * c, int block_index, direction d);
float * chunk_get_faces_offsets(chunk * c, unsigned int * instance_count);
float * chunk_get_faces_textures(chunk * c, unsigned int * instance_count, atlas * a);
float * chunk_get_faces_rotations(chunk * c, unsigned int * instance_count);
float * chunk_get_faces_scales(chunk * c, unsigned int * instance_count);

void chunk_greedymesh_face(chunk * c, int block_index, direction d, bool * is_block_meshed);
uint32_t chunk_get_mesh(chunk *c, atlas * a);
void chunk_generate_mesh(chunk *c, atlas * a);