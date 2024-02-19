#pragma once


typedef enum direction {
    DIR_START = 0,
    TOP = 0,
    BOTTOM,
    NORTH,
    SOUTH,
    EAST,
    WEST,
    DIR_COUNT
} direction;


int direction_step_value(direction d);

direction direction_reverse(direction d);
direction direction_between(int block_start, int block_end);