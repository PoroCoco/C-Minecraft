
#include <GLFW/glfw3.h>
#include <cglm/cglm.h> 

#include <assert.h>

#include <player.h>
#include <chunk.h>



// Naive raycast, need to look it up 
int raycast(player * p, chunk ** target){
    const float step = 0.1f;
    vec3 origin;
    glm_vec3_copy(p->cam->cameraPos, origin);
    vec3 direction;
    glm_vec3_copy(p->cam->cameraFront, direction);
    glm_vec3_scale(direction, step, direction);
    vec3 current_pos;
    glm_vec3_copy(origin, current_pos);

    chunk * current_chunk = p->world->center_chunk;
    current_pos[0] = chunk_norm_pos_x(current_chunk, current_pos[0]);
    current_pos[2] = chunk_norm_pos_z(current_chunk, current_pos[2]);
    for (float distance = 0.0; distance < PLAYER_RANGE; distance += step){
        // Update the current chunk
        if (current_pos[0] >= 16.0){
            current_chunk = world_get_chunk_direction(p->world, current_chunk, EAST);
        }else if (current_pos[0] < 0.0){
            current_chunk = world_get_chunk_direction(p->world, current_chunk, WEST);
        }
        if (current_pos[2] >= 16.0){
            current_chunk = world_get_chunk_direction(p->world, current_chunk, SOUTH);
        }else if (current_pos[2] < 0.0){
            current_chunk = world_get_chunk_direction(p->world, current_chunk, NORTH);
        }

        current_pos[0] = chunk_norm_pos_x(current_chunk, current_pos[0]);
        current_pos[2] = chunk_norm_pos_z(current_chunk, current_pos[2]);

        if (chunk_is_pos_inside_block(current_chunk, current_pos)){
            *target = current_chunk;
            return chunk_pos_to_index(current_pos);
        }
        
        // add step to position
        glm_vec3_add(current_pos, direction, current_pos);
    }
    

    return -1;
}

player * player_init(camera * cam, world * world){
    player * p = malloc(sizeof(*p));
    assert(p);
    p->cam = cam;
    p->world = world;
    return p;
}

void player_place_block(player *p, block b){
    // chunk * target_chunk = NULL;
    // int block_index = raycast(p, &target_chunk);
    // if (block_index != -1){
    //     assert(target_chunk);
    //     chunk_add_block(target_chunk, b, block_index);
    // }
}

void player_break_block(player *p){
    chunk * target_chunk = NULL;
    int block_index = raycast(p, &target_chunk);
    if (block_index != -1){
        assert(target_chunk);
        chunk_remove_block(target_chunk, block_index);
    }
}

void player_mouse_button_callback(player *p, int button, int action, int mods){
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        player_break_block(p);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
        player_place_block(p, block_create(1));
    }
}


void player_cleanup(player *p){
    free(p);
}