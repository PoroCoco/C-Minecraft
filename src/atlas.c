#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

    
#include <atlas.h>
#include <chunk.h>

const int TEXTURE_BLOCKS[] = {
    //TOP   BOT     NORTH   SOUTH   EAST    WEST
    1,      0,      2,      2,      2,      2,      // DIRT
    3,      3,      3,      3,      3,      3,      // STONE
    16,      16,      16,      16,      16,      16,      // WATER
};


void atlas_texture_coord_from_index(atlas * a, int atlas_index, vec2 start, vec2 end){
    int atlas_x = atlas_index % ATLAS_TEXTURE_PER_ROW; 
    int atlas_y = atlas_index / ATLAS_TEXTURE_PER_ROW;
    start[0] = (atlas_x * a->texture_resolution) / (float)a->resolution + ATLAS_PADDING;
    start[1] = (((ATLAS_TEXTURE_PER_ROW - (atlas_y+1)) * a->texture_resolution) / (float)a->resolution) + ATLAS_PADDING;
    end[0] = (((atlas_x+1) * a->texture_resolution) / (float)a->resolution) - ATLAS_PADDING;
    end[1] =  ((ATLAS_TEXTURE_PER_ROW - atlas_y) * a->texture_resolution) / (float)a->resolution - ATLAS_PADDING;
}

atlas * atlas_init(int resolution){
    atlas * a = malloc(sizeof(*a));
    assert(a);
    a->resolution = resolution;
    a->texture_resolution = resolution/ATLAS_TEXTURE_PER_ROW;
    vec2 start;
    vec2 end;
    for (size_t i = 0; i < BLOCK_COUNT * 6; i++){
        atlas_texture_coord_from_index(a, TEXTURE_BLOCKS[i], start, end);
        a->textures_coord[(i*4) + 0] = start[0];
        a->textures_coord[(i*4) + 1] = start[1];
        a->textures_coord[(i*4) + 2] = end[0];
        a->textures_coord[(i*4) + 3] = end[1];
    }
    

    return a;
}

void atlas_get_coord(atlas * a, int block_id, vec2 start, vec2 end, direction d){
    assert(block_id >= 0 && block_id < BLOCK_COUNT);
    start[0] = a->textures_coord[24*block_id + d*4 + 0];
    start[1] = a->textures_coord[24*block_id + d*4 + 1];
    end[0] = a->textures_coord[24*block_id + d*4 + 2];
    end[1] = a->textures_coord[24*block_id + d*4 + 3];
}


void atlas_cleanup(atlas *a){
    free(a);
}