#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include <cglm/cglm.h> 

#include <chunk.h>
#include <atlas.h>
#include <generation.h>


static const float vertices_face_north[] = {
    0.f, 0.f, 0.f,  0.0f, 0.0f,
    1.f, 0.f, 0.f,  1.0f, 0.0f,
    1.f,  1.f, 0.f,  1.0f, 1.0f,
    1.f,  1.f, 0.f,  1.0f, 1.0f,
    0.f,  1.f, 0.f,  0.0f, 1.0f,
    0.f, 0.f, 0.f,  0.0f, 0.0f
};

static const float vertices_face_south[] = {
    0.f, 0.f,  1.f,  0.0f, 0.0f,
    1.f, 0.f,  1.f,  1.0f, 0.0f,
    1.f,  1.f,  1.f,  1.0f, 1.0f,
    1.f,  1.f,  1.f,  1.0f, 1.0f,
    0.f,  1.f,  1.f,  0.0f, 1.0f,
    0.f, 0.f,  1.f,  0.0f, 0.0f
};


static const float vertices_face_west[] = {
    0.f, 0.f, 0.f,  0.0f, 1.0f,
    0.f, 0.f,  1.f,  0.0f, 0.0f,
    0.f,  1.f,  1.f,  1.0f, 0.0f,
    0.f,  1.f,  1.f,  1.0f, 0.0f,
    0.f,  1.f, 0.f,  1.0f, 1.0f,
    0.f, 0.f, 0.f,  0.0f, 1.0f,
};

static const float vertices_face_east[] = {
    1.f, 0.f, 0.f,  0.0f, 1.0f,
    1.f, 0.f,  1.f,  0.0f, 0.0f,
    1.f,  1.f,  1.f,  1.0f, 0.0f,
    1.f,  1.f,  1.f,  1.0f, 0.0f,
    1.f,  1.f, 0.f,  1.0f, 1.0f,
    1.f, 0.f, 0.f,  0.0f, 1.0f,
};

static const float vertices_face_top[] = {
    0.f,  1.f, 0.f,  0.0f, 1.0f,
    1.f,  1.f, 0.f,  1.0f, 1.0f,
    1.f,  1.f,  1.f,  1.0f, 0.0f,
    1.f,  1.f,  1.f,  1.0f, 0.0f,
    0.f,  1.f,  1.f,  0.0f, 0.0f,
    0.f,  1.f, 0.f,  0.0f, 1.0f
};

static const float vertices_face_bottom[] = {
    0.f, 0.f, 0.f,  0.0f, 1.0f,
    1.f, 0.f, 0.f,  1.0f, 1.0f,
    1.f, 0.f,  1.f,  1.0f, 0.0f,
    1.f, 0.f,  1.f,  1.0f, 0.0f,
    0.f, 0.f,  1.f,  0.0f, 0.0f,
    0.f, 0.f, 0.f,  0.0f, 1.0f
};

void add_face(float * vertex_data, direction d, int face_count){
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

    memcpy(vertex_data + (face_count * FACE_FLOAT_COUNT), face_vertex, FACE_BYTES);
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

chunk * chunk_init(int x, int z){
    chunk * c = malloc(sizeof(*c));
    assert(c);
    c->x = x;
    c->z = z;

    for (size_t i = 0; i < CHUNK_SIZE; i++){
        c->blocks[i] = block_create(BLOCK_AIR);
    }

    printf("init chunk %d,%d\n", x, z);
    for (size_t z = 0; z < CHUNK_Z_SIZE; z++){
        for (size_t x = 0; x < CHUNK_X_SIZE; x++){
            float noise_value = get_noise(x + c->x*CHUNK_X_SIZE , z+ c->z*CHUNK_Z_SIZE);
            int height = (int)((CHUNK_Y_SIZE - 15) * (noise_value));
            if (height < 50){
                height = 50;
            }
            for (int i = height - 1; i > 0 ; i--){
                block b;
                if ((height - i) < 4 &&  height < 90){
                    b = block_create(BLOCK_DIRT);
                }else{
                    b = block_create(BLOCK_STONE);
                }
                c->blocks[(i * CHUNK_LAYER_SIZE) + (z*CHUNK_X_SIZE) + x] = b;
            }
        }
    }

    c->vertices = NULL;
    c->vertices_count = 0;
    c->vertices_dirty = true;

    return c;
}


float * chunk_get_vertices(chunk * c, int *vertex_count, atlas * a){
    if (c->vertices_dirty){
        chunk_generate_vertices(c, a);
        c->vertices_dirty = false;
    }
    *vertex_count = c->vertices_count;
    return c->vertices;
}


float texture_coord_for_vertex_x(float x_start, float x_end, int current_vertex){
    if (current_vertex == 0){
        return x_start;
    }else if (current_vertex == 1){
        return x_end;
    }else if (current_vertex == 2){
        return x_end;
    }else if (current_vertex == 3){
        return x_end;
    }else if (current_vertex == 4){
        return x_start;
    }else if (current_vertex == 5){
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
    }else if (current_vertex == 4){
        return y_end;
    }else if (current_vertex == 5){
        return y_start;
    }
    fprintf(stderr, "???\n");
    return 0.f;
}


void chunk_generate_vertices(chunk * c, atlas * a){
    // Allocating the maximum possible vertices size. Doesn't actually allocate too much thanks to virtual memory as we'll not write on much of it  
    c->vertices = realloc(c->vertices, FACE_BYTES * 6 * CHUNK_SIZE); // Multiply by 6 as a cube have 6 faces
    assert(c->vertices);

    int face_count = 0;
    for (int block_index = 0; block_index < CHUNK_SIZE; block_index++){
        if (c->blocks[block_index].is_solid){
            for (direction d = DIR_START; d < DIR_COUNT; d++){
                if (!chunk_is_solid_direction(c, block_index, d)){
                    add_face(c->vertices, d, face_count);
                    // Shift the face coordinates and register texture
                    vec2 texture_start;
                    vec2 texture_end;
                    atlas_get_coord(a, c->blocks[block_index].id, texture_start, texture_end, d);
                    for(int i = 0; i < FACE_FLOAT_COUNT; i++){
                        if (i%ATTRIBUTE_PER_VERTEX == 0){ // x
                            c->vertices[face_count * FACE_FLOAT_COUNT + i] += chunk_block_x(block_index);
                        }else if (i%ATTRIBUTE_PER_VERTEX == 1){ // y
                            c->vertices[face_count * FACE_FLOAT_COUNT + i] += chunk_block_y(block_index);
                        }else if (i%ATTRIBUTE_PER_VERTEX == 2){ // z
                            c->vertices[face_count * FACE_FLOAT_COUNT + i] += chunk_block_z(block_index);
                        }else if (i%ATTRIBUTE_PER_VERTEX == 3){ // texture X
                            c->vertices[face_count * FACE_FLOAT_COUNT + i] = texture_coord_for_vertex_x(texture_start[0], texture_end[0], i/ATTRIBUTE_PER_VERTEX);
                        }else if (i%ATTRIBUTE_PER_VERTEX == 4){ // texture Y
                            c->vertices[face_count * FACE_FLOAT_COUNT + i] = texture_coord_for_vertex_y(texture_start[1], texture_end[1], i/ATTRIBUTE_PER_VERTEX);
                        }   
                    }
                    face_count++;
                }
            }
        }
    }
    // Now that we know the actual size we can resize it
    c->vertices = realloc(c->vertices, face_count * FACE_BYTES);
    // assert(c->vertices); //We can have no vertices in the chunk, but should be fixed
    c->vertices_count = face_count * VERTEX_PER_FACE;
}

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
    c->vertices_dirty = true; // Brute force solution, need to make a smarter update (eg only append the new block vertices and remove faces hidden by new block)
}

void chunk_remove_block(chunk * c, int index){
    if (index < 0 || index > CHUNK_SIZE){
        fprintf(stderr, "Tried to remove a block at an invalid index inside a chunk !\n");
        return;
    }

    c->blocks[index] = block_create(BLOCK_AIR);
    c->vertices_dirty = true; // Brute force solution, need to make a smarter update 
}

float chunk_norm_pos_x(chunk * c, float x){
    x = fmodf(x, (float)CHUNK_X_SIZE);
    return (float) (x > 0.0 ? x : x + 16.0);
}
float chunk_norm_pos_z(chunk * c, float z){
    z = fmodf(z, (float)CHUNK_Z_SIZE);
    return (float) (z > 0.0 ? z : z + 16.0);
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

void chunk_cleanup(chunk * c){
    free(c->vertices);
    free(c);
}