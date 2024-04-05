#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

    
#include <atlas.h>
#include <chunk.h>

/*
atlas textures id
-------
|6,7,8|
|3,4,5|
|0,1,2|
-------

Atlas indices for each texture.
*/
const int TEXTURE_BLOCKS[] = {
    //TOP   BOT     NORTH   SOUTH   EAST    WEST
    241,      240,      242,      242,      242,      242,    // DIRT
    243,      243,      243,      243,      243,      243,    // STONE
    224,      224,      224,      224,      224,      224,    // WATER
    208,      208,      208,      208,      208,      208,    // WOOD
    209,      209,      209,      209,      209,      209,    // LEAF
};


atlas * atlas_init(void){
    atlas * a = malloc(sizeof(*a));
    assert(a);

    return a;
}

int atlas_get_coord(atlas * a, int block_id, direction d){
    assert(block_id >= 0 && block_id < BLOCK_COUNT);

    return TEXTURE_BLOCKS[block_id*6 + d];
}


void atlas_cleanup(atlas *a){
    free(a);
}