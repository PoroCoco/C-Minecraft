#pragma once

#include <stdbool.h>

#define BLOCK_AIR -1

typedef enum block_id {
    BLOCK_DIRT = 0,
    BLOCK_STONE,
    BLOCK_WATER,
    BLOCK_WOOD,
    BLOCK_LEAF,
    BLOCK_COUNT
} block_id;


typedef struct block {
    block_id id;
    bool is_solid;
    bool is_transparent;
} block;



block block_create(int id);