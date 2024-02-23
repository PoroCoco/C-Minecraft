#pragma once

#include <cglm/cglm.h> 

#include <direction.h>
#include <block.h>

#define ATLAS_TEXTURE_PER_ROW 16
#define ATLAS_RESOLUTION 256
#define ATLAS_TEXTURE_RESOLUTION (ATLAS_RESOLUTION/ATLAS_TEXTURE_PER_ROW)

#define ATLAS_PADDING ((ATLAS_TEXTURE_RESOLUTION/10.0f) / (float)ATLAS_RESOLUTION)
#define ATLAS_STEP (((ATLAS_TEXTURE_RESOLUTION) / (float)ATLAS_RESOLUTION) - ATLAS_PADDING)

typedef struct atlas {
    float textures_coord[BLOCK_COUNT * 4 * 6]; // start_x,start_y,end_x,end_y and 6 faces
    int resolution;
    int texture_resolution;
} atlas;

atlas * atlas_init(int resolution);

void atlas_get_coord(atlas * a, int block_id, vec2 start, vec2 end, direction d);

void atlas_cleanup(atlas * a);
