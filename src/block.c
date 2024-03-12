#include <block.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


block block_create(uint8_t id){
    block b = {.id = id, .is_solid = true, .is_transparent = false};
    if (id == BLOCK_AIR){
        b.is_solid = false;
    }
    if (id == BLOCK_WATER){
        b.is_transparent = true;
    }
    
    return b;
}