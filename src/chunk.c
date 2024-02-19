#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include <cglm/cglm.h> 

#include <chunk.h>

// x,y,z , textCoordX,textCoordY
#define ATTRIBUTE_PER_VERTEX 5 
#define VERTEX_PER_FACE 6 
// Number of floats per face
#define FACE_FLOAT_COUNT (ATTRIBUTE_PER_VERTEX * VERTEX_PER_FACE)
// Number of bytes per face
#define FACE_BYTES (sizeof(float) * FACE_FLOAT_COUNT)


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
    0.f,  1.f,  1.f,  1.0f, 0.0f,
    0.f,  1.f, 0.f,  1.0f, 1.0f,
    0.f, 0.f, 0.f,  0.0f, 1.0f,
    0.f, 0.f, 0.f,  0.0f, 1.0f,
    0.f, 0.f,  1.f,  0.0f, 0.0f,
    0.f,  1.f,  1.f,  1.0f, 0.0f
};

static const float vertices_face_east[] = {
    1.f,  1.f,  1.f,  1.0f, 0.0f,
    1.f,  1.f, 0.f,  1.0f, 1.0f,
    1.f, 0.f, 0.f,  0.0f, 1.0f,
    1.f, 0.f, 0.f,  0.0f, 1.0f,
    1.f, 0.f,  1.f,  0.0f, 0.0f,
    1.f,  1.f,  1.f,  1.0f, 0.0f
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

bool is_solid_direction(chunk const * c, int block_index, direction d){
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
        return c->blocks[block_index + CHUNK_LAYER_SIZE].is_solid;
    case BOTTOM:
        if (block_index < CHUNK_LAYER_SIZE) return false; // Is on the bottom layer
        return c->blocks[block_index - CHUNK_LAYER_SIZE].is_solid;
    case NORTH:
        if (block_z == 0) return false; //Is on the northen side;
        return c->blocks[block_index - CHUNK_X_SIZE].is_solid;
    case SOUTH:
        if (block_z == (CHUNK_Z_SIZE-1)) return false; //Is on the south side;
        return c->blocks[block_index + CHUNK_X_SIZE].is_solid;
    case WEST:
        if (block_x == 0) return false; //Is on the west side;
        return c->blocks[block_index - 1].is_solid;
    case EAST:
        if (block_x == (CHUNK_X_SIZE-1)) return false; //Is on the east side;
        return c->blocks[block_index + 1].is_solid;
    default:
        fprintf(stderr, "Wrong direction for solid test\n");
        return false;
    }
}

chunk * chunk_init(int x, int z){
    chunk * c = malloc(sizeof(*c));
    assert(c);
    c->x = x;
    c->z = z;

    for (size_t i = 0; i < CHUNK_SIZE; i++){
        c->blocks[i] = block_create(0);
    }

    for (size_t level = 10+x+z; level < 14+x+z; level++){
        for (size_t z = 0; z < CHUNK_Z_SIZE; z++){
            for (size_t x = 0; x < CHUNK_X_SIZE; x++){
                c->blocks[(level * CHUNK_LAYER_SIZE) + (z*CHUNK_X_SIZE) + x] = block_create(1);
            }
        }
    }

    c->vertices = NULL;
    c->vertices_count = 0;
    c->vertices_dirty = true;

    return c;
}


float * chunk_get_vertices(chunk * c, int *vertex_count){
    if (c->vertices_dirty){
        printf("regen vertices\n");
        chunk_generate_vertices(c);
        c->vertices_dirty = false;
    }
    *vertex_count = c->vertices_count;
    return c->vertices;
}


void chunk_generate_vertices(chunk * c){
    // Allocating the maximum possible vertices size. Doesn't actually allocate too much thanks to virtual memory as we'll not write on much of it  
    c->vertices = realloc(c->vertices, FACE_BYTES * 6 * CHUNK_SIZE); // Multiply by 6 as a cube have 6 faces
    assert(c->vertices);

    int face_count = 0;
    for (int block_index = 0; block_index < CHUNK_SIZE; block_index++){
        if (c->blocks[block_index].is_solid){
            for (direction d = DIR_START; d < DIR_COUNT; d++){
                if (!is_solid_direction(c, block_index, d)){
                    add_face(c->vertices, d, face_count);
                    // Shift the face coordinates
                    for(int i = 0; i < FACE_FLOAT_COUNT; i++){
                        if (i%ATTRIBUTE_PER_VERTEX > 2 ) continue; // Skip texture coord attribute 
                        if (i%ATTRIBUTE_PER_VERTEX == 0){ // x
                            c->vertices[face_count * FACE_FLOAT_COUNT + i] += chunk_block_x(block_index);
                        }else if (i%ATTRIBUTE_PER_VERTEX == 1){ // y
                            c->vertices[face_count * FACE_FLOAT_COUNT + i] += chunk_block_y(block_index);
                        }else{ // z
                            c->vertices[face_count * FACE_FLOAT_COUNT + i] += chunk_block_z(block_index);
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
    printf("Adding block id %d to chunk %d,%d, at pos x,z,y %d,%d,%d\n", b.id, c->x, c->z, chunk_block_x(index), chunk_block_z(index), chunk_block_y(index));
    if (index < 0 || index > CHUNK_SIZE){
        fprintf(stderr, "Tried to add a block at an invalid index inside a chunk !\n");
        return;
    }

    if (c->blocks[index].id != 0){
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

    c->blocks[index] = block_create(0);
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