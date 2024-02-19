#include <stdbool.h>
#include <stdio.h>

#include <cglm/cglm.h> 

#include <direction.h>
#include <chunk.h>

int direction_step_value(direction d){
    switch (d)
    {
    case TOP:
        return CHUNK_LAYER_SIZE;
    case BOTTOM:
        return - CHUNK_LAYER_SIZE;
    case NORTH:
        return - CHUNK_X_SIZE;
    case SOUTH:
        return CHUNK_X_SIZE;
    case WEST:
        return - 1;
    case EAST:
        return 1;
    default:
        fprintf(stderr, "Wrong direction given to add to index\n");
        return false;
    }
}

direction direction_reverse(direction d){
    switch (d)
    {
    case TOP:
        return BOTTOM;
    case BOTTOM:
        return TOP;
    case NORTH:
        return SOUTH;
    case SOUTH:
        return NORTH;
    case WEST:
        return WEST;
    case EAST:
        return EAST;
    default:
        fprintf(stderr, "Wrong direction given to reverse\n");
        return DIR_START;
    }
}

direction direction_between(int block_start, int block_end){
    int difference = block_start - block_end;
    if (difference == 1){
        return WEST;
    }else if (difference == -1){
        return EAST;
    }else if (difference == CHUNK_X_SIZE){
        return NORTH;
    }else if (difference == -CHUNK_X_SIZE){
        return SOUTH;
    }else if (difference == CHUNK_LAYER_SIZE){
        return BOTTOM;
    }else if (difference == -CHUNK_LAYER_SIZE){
        return TOP;
    }else {
        fprintf(stderr, "The blocks given are not adjacent ! %d and %d\n", block_start, block_end);
        return DIR_START;
    }
}