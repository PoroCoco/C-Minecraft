#pragma once

#include <camera.h>
#include <world.h>
#include <block.h>


#define PLAYER_RANGE 8

typedef struct player {
    camera *cam;
    world * world;

} player;



player * player_init(camera * cam, world * world);

void player_place_block(player *p, uint8_t block_id);
void player_break_block(player *p);
void player_mouse_button_callback(player *p, int button, int action, int mods);


void player_cleanup(player *p);