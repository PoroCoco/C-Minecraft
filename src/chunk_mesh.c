#include <chunk_mesh.h>
#include <assert.h>
#include <string.h>
#include <timing.h>

bool chunk_is_solid_direction(chunk const * c, int block_index, direction d){
    if (block_index < 0 || block_index > CHUNK_SIZE){
        fprintf(stderr, "invalid block_index given : %d\n", block_index);
        return false;
    }
    int block_z = chunk_block_z(block_index);
    int block_x = chunk_block_x(block_index);
    switch (d)
    {
    case TOP:
        if (block_index > CHUNK_SIZE - CHUNK_LAYER_SIZE) return false; // Is on the top layer
        break;
    case BOTTOM:
        if ( block_index < CHUNK_LAYER_SIZE) return false; // Is on the bottom layer
        break;
    case NORTH:
        if (block_z == 0) return false; //Is on the northen side;
        break;
    case SOUTH:
        if (block_z == (CHUNK_Z_SIZE-1)) return false; //Is on the south side;
        break;
    case WEST:
        if (block_x == 0) return false; //Is on the west side;
        break;
    case EAST:
        if (block_x == (CHUNK_X_SIZE-1)) return false; //Is on the east side;
        break;
    default:
        fprintf(stderr, "Wrong direction for solid test\n");
        return false;
    }

    return block_lookup[c->block_ids[block_index + direction_step_value(d)]].is_solid;
}

void chunk_generate_faces_rotations(chunk * c){
    // Allocating the maximum possible faces_offsets size. Doesn't actually allocate too much thanks to virtual memory as we'll not write on much of it  
    c->faces_rotations = realloc(c->faces_rotations, sizeof(*c->faces_rotations) * 1 * 6 * CHUNK_SIZE); // This is a wrong max size (Multiply by 6 as a cube have 6 faces)
    assert(c->faces_rotations);
    unsigned int rotations_count = 0;

    for (int block_index = 0; block_index < CHUNK_SIZE; block_index++){
         if (block_lookup[c->block_ids[block_index]].is_solid){
            for (direction d = DIR_START; d < DIR_COUNT; d++){
                if (!chunk_is_solid_direction(c, block_index, d)){
                    c->faces_rotations[rotations_count] = (float)d;
                    rotations_count += 1;
                }
            }
        }
    }

    // Now that we know the actual size we can resize it
    assert(rotations_count == c->faces_count);
    c->faces_rotations = realloc(c->faces_rotations, sizeof(*c->faces_rotations) * 1 * c->faces_count);
}

// Returns a pointer to the rotations values for this chunk, this pointer only serve as a view and shouldn't be freed
float * chunk_get_faces_rotations(chunk * c, unsigned int *instance_count){
    if (c->rotations_dirty){
        // float time_start = (float)glfwGetTime();
        chunk_generate_faces_rotations(c);
        // printf("rotations %f\n", (float)glfwGetTime() - time_start);
        c->rotations_dirty = false;
    }
    *instance_count = c->faces_count;
    return c->faces_rotations;
}

void add_face_offset(float * faces_offsets, direction d, int face_count, int block_index){
    faces_offsets[(face_count * 3) + 0] = (float)chunk_block_x(block_index); //x
    faces_offsets[(face_count * 3) + 1] = (float)chunk_block_y(block_index); //y
    faces_offsets[(face_count * 3) + 2] = (float)chunk_block_z(block_index); //z
}

// Should be called before the generate_face_[texture/rotations] as it sets the faces_count value
void chunk_generate_faces_offsets(chunk * c){
    // Allocating the maximum possible faces_offsets size. Doesn't actually allocate too much thanks to virtual memory as we'll not write on much of it  
    c->faces_offsets = realloc(c->faces_offsets, sizeof(float) * 3 * 6 * CHUNK_SIZE); // This is a wrong max size (Multiply by 6 as a cube have 6 faces)
    assert(c->faces_offsets);
    c->faces_count = 0;
    // assert realloc return

    // printf("iterating over the chunk\n");
    for (int block_index = 0; block_index < CHUNK_SIZE; block_index++){
         if (block_lookup[c->block_ids[block_index]].is_solid){
            for (direction d = DIR_START; d < DIR_COUNT; d++){
                if (!chunk_is_solid_direction(c, block_index, d)){
                    // printf("adding face %d for block %d\n", d, block_index);
                    add_face_offset(c->faces_offsets, d, c->faces_count, block_index);
                    c->faces_count += 1;
                }
            }
        }
    }

    // Now that we know the actual size we can resize it
    c->faces_offsets = realloc(c->faces_offsets, sizeof(*c->faces_offsets) * 3 * c->faces_count);
}

// Returns a pointer to the faces offsets for this chunk, this pointer only serve as a view and shouldn't be free
float * chunk_get_faces_offsets(chunk * c, unsigned int *instance_count){
    if (c->faces_dirty){
        // float time_start = (float)glfwGetTime();
        chunk_generate_faces_offsets(c);
        // printf("faces %f\n", (float)glfwGetTime() - time_start);
        c->faces_dirty = false;
    }
    *instance_count = c->faces_count;
    return c->faces_offsets;
}

void add_face_textures(float * faces_textures, direction d, int face_count, int block_index, uint8_t id, atlas * a){
    faces_textures[face_count] = (float)atlas_get_coord(a, id, d);
}

void chunk_generate_faces_textures(chunk * c, atlas * a){
    c->faces_textures = realloc(c->faces_textures, sizeof(float) * 2 * c->faces_count);
    unsigned int texture_count = 0;

    for (int block_index = 0; block_index < CHUNK_SIZE; block_index++){
        if (block_lookup[c->block_ids[block_index]].is_solid){
            for (direction d = DIR_START; d < DIR_COUNT; d++){
                if (!chunk_is_solid_direction(c, block_index, d)){
                    add_face_textures(c->faces_textures, d, texture_count, block_index, c->block_ids[block_index], a);
                    texture_count += 1;
                }
            }
        }
    }
    assert(texture_count == c->faces_count);
}

// Returns a pointer to the texture buffer for this chunk, this pointer only serve as a view and shouldn't be free
float * chunk_get_faces_textures(chunk * c, unsigned int * instance_count, atlas * a){
    if (c->textures_dirty){
        // float time_start = (float)glfwGetTime();
        chunk_generate_faces_textures(c, a);
        // printf("textures %f\n", (float)glfwGetTime() - time_start);
        c->textures_dirty = false;
    }
    *instance_count = c->faces_count;
    return c->faces_textures;
}

float * chunk_get_faces_scales(chunk * c, unsigned int * instance_count){
    *instance_count = c->faces_count;
    return c->faces_scales;
}


void chunk_generate_mesh(chunk *c, atlas * a){
    if (c->textures_dirty || c->rotations_dirty || c->faces_dirty){
        c->faces_count = 0;
        bool *is_block_face_meshed = malloc(sizeof(*is_block_face_meshed) * CHUNK_SIZE * DIR_COUNT);
        assert(is_block_face_meshed);
        memset(is_block_face_meshed, false, CHUNK_SIZE*6);



        for (int block_index = 0; block_index < CHUNK_SIZE; block_index++){
        if (block_lookup[c->block_ids[block_index]].is_solid){
            for (direction d = DIR_START; d < DIR_COUNT; d++){
                if (!chunk_is_solid_direction(c, block_index, d) && !is_block_face_meshed[block_index*DIR_COUNT + d]){
                    
                    // Rotations
                    c->faces_rotations[c->faces_count] = (float)d;
                    // Offsets
                    add_face_offset(c->faces_offsets, d, c->faces_count, block_index);
                    // Textures
                    add_face_textures(c->faces_textures, d, c->faces_count, block_index, c->block_ids[block_index], a);

                    chunk_greedymesh_face(c, block_index, d, is_block_face_meshed);
                    c->faces_count += 1;
                }
            }
        }
        }

        c->textures_dirty = false;
        c->faces_dirty = false;
        c->rotations_dirty = false;
        free(is_block_face_meshed);
    }
}




// Give a already meshed chunk, optimize said mesh using greedy meshing to reduce number of faces
void chunk_greedymesh_face(chunk * c, int block_index, direction d, bool * is_block_face_meshed){
    is_block_face_meshed[block_index*DIR_COUNT + d] = true;
    uint8_t mesh_block_id = c->block_ids[block_index];
    int up;
    int down;
    int right;
    int left;
    int up_max;
    int right_max;
    int (*bounds_check_up)(int);
    int (*bounds_check_right)(int);

    if (d == SOUTH || d == NORTH || d == EAST || d == WEST){
        up = direction_step_value(TOP);
        down = direction_step_value(BOTTOM);
        up_max = CHUNK_Y_SIZE;
        right_max = CHUNK_X_SIZE;
        bounds_check_up = &chunk_block_y;
        if (d == EAST || d == WEST){
            bounds_check_right = &chunk_block_z;
            right = direction_step_value(SOUTH);
            left = direction_step_value(NORTH);
        }else{
            bounds_check_right = &chunk_block_x;
            right = direction_step_value(EAST);
            left = direction_step_value(WEST);
        }
    }else{
        up = direction_step_value(EAST);
        down = direction_step_value(WEST);
        up_max = CHUNK_X_SIZE;
        right_max = CHUNK_Z_SIZE;
        bounds_check_up = &chunk_block_x; 
        bounds_check_right = &chunk_block_z;
        right = direction_step_value(SOUTH);
        left = direction_step_value(NORTH);
    }

    int scale_up = 1;
    is_block_face_meshed[block_index*DIR_COUNT + d] = false;
    while(bounds_check_up(block_index) != up_max){
        int new_block_index = block_index + scale_up * up;
        if (bounds_check_up(new_block_index) <= bounds_check_up(block_index)) break;
        // if the above block face is the same as you and available
        if (c->block_ids[new_block_index] == mesh_block_id && !chunk_is_solid_direction(c, new_block_index, d) && !is_block_face_meshed[new_block_index*DIR_COUNT + d]){
            is_block_face_meshed[new_block_index*DIR_COUNT + d] = true;
            scale_up++;
        }else{
            break;
        }
    }

    int scale_right = 1;
    while(bounds_check_right(block_index) != right_max){
        bool worked = true;
        // have to check every block on the new column
        for (int height = 0; height < scale_up; height++){
            int new_block_index = block_index + height * up;
            new_block_index += scale_right * right;
            if (bounds_check_right(new_block_index) <= bounds_check_right(block_index)){
                worked = false;
                break;
            }
            if (c->block_ids[new_block_index] == mesh_block_id && !chunk_is_solid_direction(c, new_block_index, d) && !is_block_face_meshed[new_block_index*DIR_COUNT + d]){
                continue;
            }else{
                worked = false;
                break;
            }
        }
        if (!worked){
            break;
        }
        for (int height = 0; height < scale_up; height++){
            int new_block_index = block_index + height * up;
            new_block_index += scale_right * right;
            is_block_face_meshed[new_block_index*DIR_COUNT + d] = true;
        }

        scale_right++;
    }


    if (d == TOP || d == BOTTOM){
        c->faces_scales[c->faces_count*2 + 0] = (float)scale_up;
        c->faces_scales[c->faces_count*2 + 1] = (float)scale_right;
    }else{
        c->faces_scales[c->faces_count*2 + 0] = (float)scale_right;
        c->faces_scales[c->faces_count*2 + 1] = (float)scale_up;
    }
}