#pragma once

#include <cglm/cglm.h> 

#include <direction.h>
#include <block.h>

#define ATLAS_TEXTURE_PER_ROW 16

typedef struct atlas {
    float textures_coord[BLOCK_COUNT * 4 * 6]; // start_x,start_y,end_x,end_y and 6 faces
    int resolution;
    int texture_resolution;
} atlas;

atlas * atlas_init(int resolution);

void atlas_get_coord(atlas * a, int block_id, vec2 start, vec2 end, direction d);

void atlas_cleanup(atlas * a);
