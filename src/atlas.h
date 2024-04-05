#pragma once

#include <cglm/cglm.h> 

#include <direction.h>
#include <block.h>

#define ATLAS_TILE_WIDTH 16
#define ATLAS_TILE_HEIGHT 16
#define ATLAS_TILE_PER_ROW 16
#define ATLAS_TILE_PER_COL 16
#define ATLAS_TOTAL_TILES (ATLAS_TILE_PER_ROW*ATLAS_TILE_PER_COL)
#define ATLAS_TILE_RESOLUTION (ATLAS_TILE_HEIGHT*ATLAS_TILE_WIDTH)

typedef struct atlas {
    int tmp;
} atlas;

atlas * atlas_init(void);

int atlas_get_coord(atlas * a, int block_id, direction d);

void atlas_cleanup(atlas * a);
