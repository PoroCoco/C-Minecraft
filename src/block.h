#pragma once

#include <stdbool.h>
#include <stdint.h>


typedef enum block_id {
    BLOCK_DIRT = 0,
    BLOCK_STONE,
    BLOCK_WATER,
    BLOCK_WOOD,
    BLOCK_LEAF,
    BLOCK_COUNT
} block_id;

#define BLOCK_AIR BLOCK_COUNT

typedef struct block {
    block_id id;
    bool is_solid;
    bool is_transparent;
} block;

static block block_lookup[BLOCK_COUNT+1] = {
    {.id = 0, .is_solid = true, .is_transparent = false},
    {.id = 1, .is_solid = true, .is_transparent = false},
    {.id = 2, .is_solid = true, .is_transparent = true},
    {.id = 3, .is_solid = true, .is_transparent = false},
    {.id = 4, .is_solid = true, .is_transparent = false},
    {.id = 5, .is_solid = false, .is_transparent = false},
};


block block_create(uint8_t id);