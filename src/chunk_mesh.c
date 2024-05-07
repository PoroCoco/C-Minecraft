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

void add_face_offset(uint32_t * faces_packed_data, direction d, int face_count, int block_index){
    faces_packed_data[face_count*2+0] |= ((uint8_t)chunk_block_x(block_index) << 0); //x
    faces_packed_data[face_count*2+0] |= ((uint8_t)chunk_block_y(block_index) << 6); //y
    faces_packed_data[face_count*2+0] |= ((uint8_t)chunk_block_z(block_index) << 13); //z
}

void add_face_textures(uint32_t * faces_packed_data, direction d, int face_count, int block_index, uint8_t id, atlas * a){
    faces_packed_data[face_count*2+0] |= ((uint8_t)atlas_get_coord(a, id, d) << 19);
}

void chunk_generate_mesh(chunk *c, atlas * a){
    c->faces_count = 0;
    bool *is_block_face_meshed = malloc(sizeof(*is_block_face_meshed) * CHUNK_SIZE * DIR_COUNT);
    assert(is_block_face_meshed);
    memset(is_block_face_meshed, false, CHUNK_SIZE*6);

    for (int block_index = 0; block_index < CHUNK_SIZE; block_index++){
    if (block_lookup[c->block_ids[block_index]].is_solid){
        for (direction d = DIR_START; d < DIR_COUNT; d++){
            if (!chunk_is_solid_direction(c, block_index, d) && !is_block_face_meshed[block_index*DIR_COUNT + d]){
                // Clearing the bits
                c->faces_packed_data[c->faces_count*2+0] &= 0;
                c->faces_packed_data[c->faces_count*2+1] &= 0;
                // Offsets
                add_face_offset(c->faces_packed_data, d, c->faces_count, block_index);
                // Textures
                add_face_textures(c->faces_packed_data, d, c->faces_count, block_index, c->block_ids[block_index], a);
                // Rotations
                c->faces_packed_data[c->faces_count*2+0] |= ((uint8_t)d << 27);

                chunk_greedymesh_face(c, block_index, d, is_block_face_meshed);
                c->faces_count += 1;
            }
        }
    }
    }


    c->mesh_dirty = false;
    free(is_block_face_meshed);
}

uint32_t chunk_get_mesh(chunk *c, atlas * a){
    if (c->mesh_dirty){
        chunk_generate_mesh(c, a);
    }
    return c->faces_count;
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

    // Cast from (1-64)/(1-128) to (0-63)/(0-127) for packing in bits. Shader does the inverse
    scale_right--;
    scale_up--;
    assert(scale_right < 64);
    assert(scale_up < 128);
    if (d == TOP || d == BOTTOM){
        c->faces_packed_data[c->faces_count*2+1] |= ((uint8_t)scale_up << 0);
        c->faces_packed_data[c->faces_count*2+1] |= ((uint8_t)scale_right << 7);
    }else{
        c->faces_packed_data[c->faces_count*2+1] |= ((uint8_t)scale_right << 0); 
        c->faces_packed_data[c->faces_count*2+1] |= ((uint8_t)scale_up << 7);
    }
}