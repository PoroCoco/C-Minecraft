#include <block.h>
#include <stdlib.h>
#include <string.h>

block block_create(int id){
    block b = {.id = id, .is_solid = true};
    if (id == 0){
        b.is_solid = false;
    }
    
    return b;
}