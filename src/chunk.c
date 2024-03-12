#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include <GLFW/glfw3.h>
#include <cglm/cglm.h> 

#include <chunk.h>
#include <atlas.h>
#include <generation.h>


static const float vertices_face_north[] = {
    0.f, 0.f, 0.f,
    1.f, 0.f, 0.f,
    1.f,  1.f, 0.f,
    0.f,  1.f, 0.f,
};

static const float vertices_face_south[] = {
    0.f, 0.f,  1.f,
    1.f, 0.f,  1.f,
    1.f,  1.f,  1.f,
    0.f,  1.f,  1.f,
};


static const float vertices_face_west[] = {
    0.f, 0.f, 0.f,
    0.f, 0.f,  1.f,
    0.f,  1.f,  1.f,
    0.f,  1.f, 0.f,
};

static const float vertices_face_east[] = {
    1.f, 0.f, 0.f,
    1.f, 0.f,  1.f,
    1.f,  1.f,  1.f,
    1.f,  1.f, 0.f,
};

static const float vertices_face_top[] = {
    0.f,  1.f, 0.f,
    1.f,  1.f, 0.f,
    1.f,  1.f,  1.f,
    0.f,  1.f,  1.f,
};

static const float vertices_face_bottom[] = {
    0.f, 0.f, 0.f,
    1.f, 0.f, 0.f,
    1.f, 0.f,  1.f,
    0.f, 0.f,  1.f,
};

void add_face_vertices(float * vertex_data, direction d, int face_count){
    float const * face_vertex;

    switch (d)
    {
    case TOP:
        face_vertex = vertices_face_top;
        break;
    case BOTTOM:
        face_vertex = vertices_face_bottom;
        break;
    case SOUTH:
        face_vertex = vertices_face_south;
        break;
    case NORTH:
        face_vertex = vertices_face_north;
        break;
    case WEST:
        face_vertex = vertices_face_west;
        break;
    case EAST:
        face_vertex = vertices_face_east;
        break;
    default:
        face_vertex = vertices_face_top;
        fprintf(stderr, "Wrong direction for face adding\n");
        break;
    }

    memcpy(vertex_data + (face_count * 3 * 4), face_vertex, 3 * 4 * sizeof(float));
}


void chunk_generate_faces_rotations(chunk * c){
    // Allocating the maximum possible faces_offsets size. Doesn't actually allocate too much thanks to virtual memory as we'll not write on much of it  
    c->faces_rotations = realloc(c->faces_rotations, sizeof(*c->faces_rotations) * 1 * 6 * CHUNK_SIZE); // This is a wrong max size (Multiply by 6 as a cube have 6 faces)
    assert(c->faces_rotations);
    unsigned int rotations_count = 0;

    for (int block_index = 0; block_index < CHUNK_SIZE; block_index++){
        if (c->blocks[block_index].is_solid){
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

void chunk_index_to_pos(int block_index, vec3 pos){
    pos[0] = (float)chunk_block_x(block_index);
    pos[1] = (float)chunk_block_y(block_index);
    pos[2] = (float)chunk_block_z(block_index);
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
        if (c->blocks[block_index].is_solid){
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

void add_face_textures(float * faces_textures, direction d, int face_count, int block_index, block_id id, atlas * a){
    vec2 start, end;
    atlas_get_coord(a, id, start, end, d);
    faces_textures[(face_count * 2) + 0] = start[0];
    faces_textures[(face_count * 2) + 1] = start[1];

}

void chunk_generate_faces_textures(chunk * c, atlas * a){
    c->faces_textures = realloc(c->faces_textures, sizeof(float) * 2 * c->faces_count);
    unsigned int texture_count = 0;

    for (int block_index = 0; block_index < CHUNK_SIZE; block_index++){
        if (c->blocks[block_index].is_solid){
            for (direction d = DIR_START; d < DIR_COUNT; d++){
                if (!chunk_is_solid_direction(c, block_index, d)){
                    add_face_textures(c->faces_textures, d, texture_count, block_index, c->blocks[block_index].id, a);
                    texture_count += 1;
                }
            }
        }
    }
    assert(texture_count == c->faces_count);
}

// Returns a pointer to the texture buffer for this chunk, this pointer only serve as a view and shouldn't be free
float * chunk_get_textures(chunk * c, unsigned int * faces_count, atlas * a){
    if (c->textures_dirty){
        // float time_start = (float)glfwGetTime();
        chunk_generate_faces_textures(c, a);
        // printf("textures %f\n", (float)glfwGetTime() - time_start);
        c->textures_dirty = false;
    }
    *faces_count = c->faces_count;
    return c->faces_textures;
}

size_t chunk_sizeof(chunk * c){
    size_t size = 0;
    size += sizeof(c->blocks);
    size += 2 * c->faces_count * sizeof(*(c->faces_textures));
    size += 3 * c->faces_count * sizeof(*(c->faces_offsets));
    size += 1 * c->faces_count * sizeof(*(c->faces_rotations));
    return size;
}



int chunk_block_z(int block_index){
    return (block_index % CHUNK_LAYER_SIZE) / CHUNK_X_SIZE;
}

int chunk_block_y(int block_index){
    return block_index / CHUNK_LAYER_SIZE;
}

int chunk_block_x(int block_index){
    return (block_index % CHUNK_LAYER_SIZE) % CHUNK_X_SIZE;
}

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

    return c->blocks[block_index + direction_step_value(d)].is_solid;
}

void chunk_generate_tree(chunk *c, int x, int z, int y){
    int tree_height = 5;    
    for (size_t i = 0; i < tree_height; i++){
        c->blocks[((i+y) * CHUNK_LAYER_SIZE) + (z*CHUNK_X_SIZE) + x] = block_create(BLOCK_WOOD);
        if (i>2){
            for (direction d = DIR_START; d < DIR_COUNT; d++){
                if (d == BOTTOM || d == TOP) continue;
                int tree_trunk_pos = ((i+y) * CHUNK_LAYER_SIZE) + (z*CHUNK_X_SIZE) + x;
                int leaf_pos = tree_trunk_pos + direction_step_value(d);
                c->blocks[leaf_pos] = block_create(BLOCK_LEAF);
            }
        }
    }
    c->blocks[((tree_height+y) * CHUNK_LAYER_SIZE) + (z*CHUNK_X_SIZE) + x] = block_create(BLOCK_LEAF);
    for (direction d = DIR_START; d < DIR_COUNT; d++){
        if (d == BOTTOM || d == TOP) continue;
        int tree_trunk_pos = ((tree_height+y) * CHUNK_LAYER_SIZE) + (z*CHUNK_X_SIZE) + x;
        int leaf_pos = tree_trunk_pos + direction_step_value(d);
        c->blocks[leaf_pos] = block_create(BLOCK_LEAF);
    }
    c->blocks[((tree_height+y+1) * CHUNK_LAYER_SIZE) + (z*CHUNK_X_SIZE) + x] = block_create(BLOCK_LEAF);
}

chunk * chunk_init(int x, int z){
    chunk * c = malloc(sizeof(*c));
    assert(c);
    c->x = x;
    c->z = z;

    for (size_t i = 0; i < CHUNK_SIZE; i++){
        c->blocks[i] = block_create(BLOCK_AIR);
    }

    // printf("init chunk %d,%d\n", x, z);
    for (int z = 0; z < CHUNK_Z_SIZE; z++){
        for (int x = 0; x < CHUNK_X_SIZE; x++){
            float noise_value = get_noise(x + c->x*CHUNK_X_SIZE , z+ c->z*CHUNK_Z_SIZE);
            int height = (int)((CHUNK_Y_SIZE - 15) * (noise_value));
            if (height < 30){
                height = 30;
                for (int i = height - 1; i > 0 ; i--){
                    block b;
                    if ((height-i) < 30){
                        b = block_create(BLOCK_WATER);
                    }else if ((height-i) < height){
                        b = block_create(BLOCK_STONE);
                    }
                    c->blocks[(i * CHUNK_LAYER_SIZE) + (z*CHUNK_X_SIZE) + x] = b;
                }
            }else{
                for (int i = height - 1; i > 0 ; i--){
                    block b;
                    if ((height - i) < 4 &&  height < 90){
                        b = block_create(BLOCK_DIRT);
                    }else{
                        b = block_create(BLOCK_STONE);
                    }
                    c->blocks[(i * CHUNK_LAYER_SIZE) + (z*CHUNK_X_SIZE) + x] = b;
                }
                float tree_noise = get_noise_tree(x + c->x*CHUNK_X_SIZE , z+ c->z*CHUNK_Z_SIZE);
                // printf("%f\t", tree_noise);
                if (tree_noise > 0.85){ 
                    chunk_generate_tree(c, x, z, height);               
                } 
            }
            
        }
        // printf("\n");
    }
    // c->blocks[0] = block_create(BLOCK_DIRT);
    c->timestap_generation = (float)glfwGetTime();

    c->faces_offsets = NULL;
    c->faces_count = 0;
    c->faces_dirty = true;
    c->faces_textures = NULL;
    c->textures_dirty = true;
    c->faces_rotations = NULL;
    c->rotations_dirty = true;
    c->in_frustum = false;
    return c;
}



float texture_coord_for_vertex_x(float x_start, float x_end, int current_vertex){
    if (current_vertex == 0){
        return x_start;
    }else if (current_vertex == 1){
        return x_end;
    }else if (current_vertex == 2){
        return x_end;
    }else if (current_vertex == 3){
        return x_start;
    }
    fprintf(stderr, "???\n");
    return 0.f;
}

float texture_coord_for_vertex_y(float y_start, float y_end, int current_vertex){
    if (current_vertex == 0){
        return y_start;
    }else if (current_vertex == 1){
        return y_start;
    }else if (current_vertex == 2){
        return y_end;
    }else if (current_vertex == 3){
        return y_end;
    }
    
    fprintf(stderr, "???\n");
    return 0.f;
}

/*
     ____________________________________________________________________________________________
    |______________________________________________block_index______________________             |
    |______________________________________ directions                              |            |
    |              face vertices          |                                         |            |
    [vertex0Dir0Block0,..,vertex3Dir0Block0;..;vertex0Dir5Block0,..,vertex3Dir5Block0;...........;.......]
    4 vertex per direction, 6 directions per block_index, CHUNK_SIZE block_indices
*/
// float * chunk_generate_static_mesh(unsigned int *vertice_count){
//     float * vertices = malloc(FACE_BYTES * 6 * CHUNK_SIZE); // Multiply by 6 as a cube have 6 faces
//     assert(vertices);

//     int face_count = 0;
//     for (int block_index = 0; block_index < CHUNK_SIZE; block_index++){
//         for (direction d = DIR_START; d < DIR_COUNT; d++){
//             add_face_vertices(vertices, d, face_count);
//             // Shift the face coordinates
//             for(int i = 0; i < FACE_FLOAT_COUNT; i++){
//                 if (i%ATTRIBUTE_PER_VERTEX == 0){       // x
//                     vertices[face_count * FACE_FLOAT_COUNT + i] += chunk_block_x(block_index);
//                 }else if (i%ATTRIBUTE_PER_VERTEX == 1){ // y
//                     vertices[face_count * FACE_FLOAT_COUNT + i] += chunk_block_y(block_index);
//                 }else if (i%ATTRIBUTE_PER_VERTEX == 2){ // z
//                     vertices[face_count * FACE_FLOAT_COUNT + i] += chunk_block_z(block_index);
//                 }
//             }
//             face_count++;
//         }
//     }
//     *vertice_count = face_count * VERTEX_PER_FACE;
//     return vertices;
// } 


// Returns a pointer to the element buffer for this chunk, this pointer only serve as a view and shouldn't be free
// unsigned int * chunk_get_elements(chunk * c, unsigned int *vertex_count){
//     if (c->elements_dirty){
//         chunk_generate_elements_buffer(c);
//         c->elements_dirty = false;
//     }
//     *vertex_count = c->elements_count;
//     return c->elements;
// }

// void add_face_elements(unsigned int * elements_data, direction d, int elements_count, int block_index){
//     elements_data[elements_count + 0] = block_index * (DIR_COUNT * VERTEX_PER_FACE) + d * (VERTEX_PER_FACE) + 0;
//     elements_data[elements_count + 1] = block_index * (DIR_COUNT * VERTEX_PER_FACE) + d * (VERTEX_PER_FACE) + 1;
//     elements_data[elements_count + 2] = block_index * (DIR_COUNT * VERTEX_PER_FACE) + d * (VERTEX_PER_FACE) + 2;
//     elements_data[elements_count + 3] = block_index * (DIR_COUNT * VERTEX_PER_FACE) + d * (VERTEX_PER_FACE) + 2;
//     elements_data[elements_count + 4] = block_index * (DIR_COUNT * VERTEX_PER_FACE) + d * (VERTEX_PER_FACE) + 3;
//     elements_data[elements_count + 5] = block_index * (DIR_COUNT * VERTEX_PER_FACE) + d * (VERTEX_PER_FACE) + 0;
// }

// void chunk_generate_elements_buffer(chunk * c){
//     // Allocating the maximum possible vertices size. Doesn't actually allocate too much thanks to virtual memory as we'll not write on much of it  
//     c->elements = realloc(c->elements, FACE_BYTES * 6 * CHUNK_SIZE); // This is a wrong max size (Multiply by 6 as a cube have 6 faces)
//     assert(c->elements);

//     int elements_count = 0;
//     for (int block_index = 0; block_index < CHUNK_SIZE; block_index++){
//         if (c->blocks[block_index].is_solid){
//             for (direction d = DIR_START; d < DIR_COUNT; d++){
//                 if (!chunk_is_solid_direction(c, block_index, d)){
//                     add_face_elements(c->elements, d, elements_count, block_index);
//                     elements_count += 6; // 2 triangles
//                 }
//             }
//         }
//     }
//     // printf("elem %d\n", elements_count);
//     // Now that we know the actual size we can resize it
//     c->elements = realloc(c->elements, elements_count * sizeof(*(c->elements)));
//     // assert(c->elements); //We can have no vertices in the chunk, but should be fixed
//     c->elements_count = elements_count;
//     printf("chunk face count %d\n", c->elements_count/6);
//     // printf("chunk %d,%d size %zu\n", c->x, c->z, chunk_sizeof(c));
// }

// ToDo : OPTI
void chunk_add_block(chunk * c, block b, int index){
    if (index < 0 || index > CHUNK_SIZE){
        fprintf(stderr, "Tried to add a block at an invalid index inside a chunk !\n");
        return;
    }
    printf("Adding block id %d to chunk %d,%d, at pos x,z,y %d,%d,%d\n", b.id, c->x, c->z, chunk_block_x(index), chunk_block_z(index), chunk_block_y(index));

    if (c->blocks[index].id != BLOCK_AIR){
        fprintf(stderr, "Tried to replace an existing block inside a chunk ! Was this expected ?\n");
    }

    c->blocks[index] = b;
    c->textures_dirty = true; // Brute force solution, need to make a smarter update 
    c->faces_dirty = true; // Brute force solution, need to make a smarter update 
    c->rotations_dirty = true; // Brute force solution, need to make a smarter update 
}

// ToDo : OPTI
void chunk_remove_block(chunk * c, int index){
    if (index < 0 || index > CHUNK_SIZE){
        fprintf(stderr, "Tried to remove a block at an invalid index inside a chunk ! (%d)\n", index);
        return;
    }

    c->blocks[index] = block_create(BLOCK_AIR);
    c->textures_dirty = true; // Brute force solution, need to make a smarter update 
    c->faces_dirty = true; // Brute force solution, need to make a smarter update 
    c->rotations_dirty = true; // Brute force solution, need to make a smarter update 
}

float chunk_norm_pos_x(chunk * c, float x){
    x = fmodf(x, (float)CHUNK_X_SIZE);
    return (float) (x > 0.0 ? x : x + (float)CHUNK_X_SIZE);
}
float chunk_norm_pos_z(chunk * c, float z){
    z = fmodf(z, (float)CHUNK_Z_SIZE);
    return (float) (z > 0.0 ? z : z + (float)CHUNK_Z_SIZE);
}

int chunk_pos_to_index(vec3 pos){
    return (int) (floorf(pos[0]) + floorf(pos[2]) * CHUNK_X_SIZE + floorf(pos[1]) * CHUNK_LAYER_SIZE); 
}

bool chunk_is_pos_inside_block(chunk const * c, vec3 pos){
    int block_index = chunk_pos_to_index(pos);
    if (block_index < 0 && block_index > CHUNK_SIZE){
        return false;
    }
    return c->blocks[block_index].is_solid;
}


uint64_t chunk_get_id_pos(int x, int z){
    return (uint64_t)(((uint64_t)x << 32) | (uint64_t)(unsigned int)z);
}

uint64_t chunk_get_id(chunk * c){
    return chunk_get_id_pos(c->x, c->z);
}

void chunk_cleanup(chunk * c){
    free(c->faces_rotations);
    free(c->faces_textures);
    free(c->faces_offsets);
    free(c);
}