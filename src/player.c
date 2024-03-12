
#include <GLFW/glfw3.h>
#include <cglm/cglm.h> 

#include <assert.h>

#include <player.h>
#include <direction.h>
#include <chunk.h>



// Return the index of the first block hitten and fill the final chunk with the chunk the block is part of 
int raycast_break(player * p, chunk ** final_chunk){
    const float step = 0.05f;
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
        if (current_pos[0] > (float)(CHUNK_X_SIZE)){
            current_chunk = world_get_chunk_direction(p->world, current_chunk, EAST);
        }else if (current_pos[0] < 0.0){
            current_chunk = world_get_chunk_direction(p->world, current_chunk, WEST);
        }
        if (current_pos[2] > (float)(CHUNK_Z_SIZE)){
            current_chunk = world_get_chunk_direction(p->world, current_chunk, SOUTH);
        }else if (current_pos[2] < 0.0){
            current_chunk = world_get_chunk_direction(p->world, current_chunk, NORTH);
        }

        current_pos[0] = chunk_norm_pos_x(current_chunk, current_pos[0]);
        current_pos[2] = chunk_norm_pos_z(current_chunk, current_pos[2]);

        if (chunk_is_pos_inside_block(current_chunk, current_pos)){
            *final_chunk = current_chunk;
            int block_hit_index = chunk_pos_to_index(current_pos);
            return block_hit_index;
        }
        // add step to position
        glm_vec3_add(current_pos, direction, current_pos);
    }

    return -1;
}

// Return the index of the block to the side of the block hitten and fill the final chunk with the chunk the block is part of. Also update the direction to the side hit (from the block pov).
int raycast_adjacent(player * p, chunk ** final_chunk, direction *d){
    const float step = 0.05f;
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
    bool previous_chunk_crossed = false;
    for (float distance = 0.0; distance < PLAYER_RANGE; distance += step){
        // Update the current chunk
        if (current_pos[0] >= (float)(CHUNK_X_SIZE)){
            current_chunk = world_get_chunk_direction(p->world, current_chunk, EAST);
            previous_chunk_crossed = true;
        }else if (current_pos[0] < 0.0){
            current_chunk = world_get_chunk_direction(p->world, current_chunk, WEST);
            previous_chunk_crossed = true;
        }
        if (current_pos[2] >= (float)(CHUNK_Z_SIZE)){
            current_chunk = world_get_chunk_direction(p->world, current_chunk, SOUTH);
            previous_chunk_crossed = true;
        }else if (current_pos[2] < 0.0){
            current_chunk = world_get_chunk_direction(p->world, current_chunk, NORTH);
            previous_chunk_crossed = true;
        }

        current_pos[0] = chunk_norm_pos_x(current_chunk, current_pos[0]);
        current_pos[2] = chunk_norm_pos_z(current_chunk, current_pos[2]);

        if (chunk_is_pos_inside_block(current_chunk, current_pos)){
            *final_chunk = current_chunk;
            vec3 previous_pos;
            glm_vec3_sub(current_pos, direction, previous_pos);
            int previous_block_index = chunk_pos_to_index(previous_pos);
            int block_hit_index = chunk_pos_to_index(current_pos);
            if (previous_block_index == block_hit_index){
                return -1;
            }
            *d = direction_between(block_hit_index, previous_block_index);

            if (previous_chunk_crossed){ // it just works
                *final_chunk = world_get_chunk_direction(p->world, current_chunk, *d);
                if (final_chunk == NULL) return -1;
                int displacement = 0;
                if (*d == NORTH){
                    displacement = CHUNK_LAYER_SIZE;
                }else if (*d == SOUTH){
                    displacement = -CHUNK_LAYER_SIZE;
                }else if (*d == WEST){
                    displacement = CHUNK_X_SIZE;
                }else if (*d == EAST){
                    displacement = -CHUNK_X_SIZE;
                }
                return previous_block_index + displacement;
            }

            return block_hit_index + direction_step_value(*d);
        }
        previous_chunk_crossed = false;
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

void player_place_block(player *p, uint8_t block_id){
    chunk * target_chunk = NULL;
    direction d;
    int block_index = raycast_adjacent(p, &target_chunk, &d);
    if (block_index != -1){
        assert(target_chunk);
        chunk_add_block(target_chunk, block_id, block_index);
    }
}

void player_break_block(player *p){
    chunk * target_chunk = NULL;
    int block_index = raycast_break(p, &target_chunk);
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
        player_place_block(p, BLOCK_DIRT);
    }
}


void player_cleanup(player *p){
    free(p);
}