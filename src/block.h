#pragma once

#include <stdbool.h>

typedef struct block {
    unsigned int id;
    bool is_solid;
} block;

block block_create(int id);