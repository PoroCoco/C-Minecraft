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

#ifdef _WIN32
#include <windows.h>
#endif


float (*generator_get_noise)(int, int ) = NULL;
float (*generator_get_noise_tree)(int, int ) = NULL;
void* generator_lib_handle = NULL;

// 0 unloaded, 1 loaded
int generator_lib_state = 0; 

void chunk_generate_tree(chunk *c, int x, int z, int y){
    if (x == 0 || x == CHUNK_X_SIZE-1 || z == 0 || z == CHUNK_Z_SIZE-1 ) return;
    int tree_height = 5;    
    for (int i = 0; i < tree_height; i++){
        c->block_ids[((i+y) * CHUNK_LAYER_SIZE) + (z*CHUNK_X_SIZE) + x] = BLOCK_WOOD;
        if (i>2){
            for (direction d = DIR_START; d < DIR_COUNT; d++){
                if (d == BOTTOM || d == TOP) continue;
                int tree_trunk_pos = (int)((i+y) * CHUNK_LAYER_SIZE) + (z*CHUNK_X_SIZE) + x;
                int leaf_pos = tree_trunk_pos + direction_step_value(d);
                c->block_ids[leaf_pos] = BLOCK_LEAF;
            }
        }
    }
    c->block_ids[((tree_height+y) * CHUNK_LAYER_SIZE) + (z*CHUNK_X_SIZE) + x] = BLOCK_LEAF;
    for (direction d = DIR_START; d < DIR_COUNT; d++){
        if (d == BOTTOM || d == TOP) continue;
        int tree_trunk_pos = (int)((tree_height+y) * CHUNK_LAYER_SIZE) + (z*CHUNK_X_SIZE) + x;
        int leaf_pos = tree_trunk_pos + direction_step_value(d);
        c->block_ids[leaf_pos] = BLOCK_LEAF;
    }
    c->block_ids[((tree_height+y+1) * CHUNK_LAYER_SIZE) + (z*CHUNK_X_SIZE) + x] = BLOCK_LEAF;
}

void chunk_hotreload_genetor(bool reload){
    static const char * generator_lib_name = "libworld_gen.dll";

    if (reload && generator_lib_state == 1){
        FreeLibrary((HMODULE)generator_lib_handle);
        generator_lib_state = 0;
        printf("unloaded world generator\n");
        return;
    }

    generator_lib_handle = LoadLibrary(generator_lib_name);
    if (generator_lib_handle == NULL) {
        fprintf(stderr, "Failed to load the world generator\n");
        return;
    }
    printf("Loaded world generator\n");
    generator_lib_state = 1;

    generator_get_noise = GetProcAddress(generator_lib_handle, "get_noise");
    if (generator_get_noise == NULL) {
        fprintf(stderr, "Failed to load the world generator function : get_noise\n");
        FreeLibrary(generator_lib_handle);
        return;
    }

    generator_get_noise_tree = GetProcAddress(generator_lib_handle, "get_noise_tree");
    if (generator_get_noise_tree == NULL) {
        fprintf(stderr, "Failed to load the world generator function : get_noise_tree\n");
        FreeLibrary(generator_lib_handle);
        return;
    }
}

chunk * chunk_init(int x, int z){
    chunk * c = malloc(sizeof(*c));
    assert(c);
    c->x = x;
    c->z = z;

    for (size_t i = 0; i < CHUNK_SIZE; i++){
        c->block_ids[i] = BLOCK_AIR;
    }
    c->ready = false;

    static int water_level = 5;
    static int moutain_start = 90;

    // printf("init chunk %d,%d\n", x, z);
    for (int z = 0; z < CHUNK_Z_SIZE; z++){
        for (int x = 0; x < CHUNK_X_SIZE; x++){
            float noise_value = (*generator_get_noise)(x + c->x*CHUNK_X_SIZE , z+ c->z*CHUNK_Z_SIZE);
            int height = (int)((CHUNK_Y_SIZE - 5) * (noise_value));
            if (height < water_level){
                height = water_level;
                for (int i = height - 1; i > 0 ; i--){
                    uint8_t b;
                    if ((height-i) < water_level){
                        b = BLOCK_WATER;
                    }else if ((height-i) < height){
                        b = BLOCK_STONE;
                    }
                    c->block_ids[(i * CHUNK_LAYER_SIZE) + (z*CHUNK_X_SIZE) + x] = b;
                }
            }else{
                for (int i = height - 1; i > 0 ; i--){
                    uint8_t b;
                    if ((height - i) < 4 &&  height < moutain_start){
                        b = BLOCK_DIRT;
                    }else{
                        b = BLOCK_STONE;
                    }
                    c->block_ids[(i * CHUNK_LAYER_SIZE) + (z*CHUNK_X_SIZE) + x] = b;
                }
                if (height < moutain_start){
                    float tree_noise = (*generator_get_noise_tree)(x + c->x*CHUNK_X_SIZE , z+ c->z*CHUNK_Z_SIZE);
                    // printf("%f\t", tree_noise);
                    if (tree_noise > 0.85){ 
                        chunk_generate_tree(c, x, z, height);               
                    } 
                }
            }
            
        }
        // printf("\n");
    }
    c->timestap_generation = (float)glfwGetTime();

    c->faces_count = 0;
    c->faces_packed_data = NULL;
    c->faces_packed_data = realloc(c->faces_packed_data, sizeof(c->faces_packed_data) * PACKED_4BYTES_COUNT * 6 * MAX_FACE_IN_CHUNK/4); // This is a wrong max size (Multiply by 6 as a cube have 6 faces)
    c->mesh_dirty = true;

    c->in_frustum = false;

    return c;
}

size_t chunk_sizeof(chunk * c){
    size_t size = 0;
    size += sizeof(c->block_ids);
    size += 2 * c->faces_count * sizeof(*(c->faces_packed_data));
    return size;
}

inline int chunk_block_z(int block_index){
    return (block_index % CHUNK_LAYER_SIZE) / CHUNK_X_SIZE;
}

inline int chunk_block_y(int block_index){
    return block_index / CHUNK_LAYER_SIZE;
}

inline int chunk_block_x(int block_index){
    return (block_index % CHUNK_LAYER_SIZE) % CHUNK_X_SIZE;
}

void chunk_index_to_pos(int block_index, vec3 pos){
    pos[0] = (float)chunk_block_x(block_index);
    pos[1] = (float)chunk_block_y(block_index);
    pos[2] = (float)chunk_block_z(block_index);
}


// ToDo : OPTI
void chunk_add_block(chunk * c, uint8_t block_id, int index){
    if (index < 0 || index > CHUNK_SIZE){
        fprintf(stderr, "Tried to add a block at an invalid index inside a chunk !\n");
        return;
    }
    printf("Adding block id %d to chunk %d,%d, at pos x,z,y %d,%d,%d\n", block_id, c->x, c->z, chunk_block_x(index), chunk_block_z(index), chunk_block_y(index));

    if (c->block_ids[index] != BLOCK_AIR){
        fprintf(stderr, "Tried to replace an existing block inside a chunk ! Was this expected ?\n");
    }

    c->block_ids[index] = block_id;
    c->mesh_dirty = true; // Brute force solution, need to make a smarter update 
}

// ToDo : OPTI
void chunk_remove_block(chunk * c, int index){
    if (index < 0 || index > CHUNK_SIZE){
        fprintf(stderr, "Tried to remove a block at an invalid index inside a chunk ! (%d)\n", index);
        return;
    }

    c->block_ids[index] = BLOCK_AIR;
    c->mesh_dirty = true; // Brute force solution, need to make a smarter update 
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
    return block_lookup[c->block_ids[block_index]].is_solid;
}


uint64_t chunk_get_id_pos(int x, int z){
    return (uint64_t)(((uint64_t)x << 32) | (uint64_t)(unsigned int)z);
}

uint64_t chunk_get_id(chunk * c){
    return chunk_get_id_pos(c->x, c->z);
}

void chunk_cleanup(chunk * c){
    free(c->faces_packed_data);
    free(c);
}