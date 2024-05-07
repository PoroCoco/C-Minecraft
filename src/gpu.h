#pragma once

#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdint.h>
#include <chunk.h>
#include <camera.h>
#include <chunk_mesh.h>
#include <atlas.h>
#include <hashtable.h>
#include <queue.h>
#include <threads.h>

#define DEBUG_GL(command) do { \
    command; \
    GLenum error = glGetError(); \
    if (error != GL_NO_ERROR) { \
        fprintf(stderr, "OpenGL Error at %s:%d - Code %d\n", __FILE__, __LINE__, error); \
    } \
} while(0)


#define WIDTH 2300
#define HEIGHT 1200

typedef struct gpu {
    queue * command_queue;
    atlas * atlas;
    htb * shaders;

    // OpenGL Objects
    // Chunks
    uint32_t VAO[TOTAL_CHUNKS];
    uint32_t VBO_face;
    uint32_t IBA[TOTAL_CHUNKS];   // Instance Buffer Attributes
    uint32_t instances_count[TOTAL_CHUNKS];

    // Skybox
    uint32_t skybox_vao;
    uint32_t skybox_vbo;
    uint32_t skybox_cubemap_texture;

    mtx_t mutex; // OpenGL does work well with multiple threads, therefore only 1 thread is assigned to do all this stuff
    mtx_t draw_mutex; // mutex locked at beginning of drawing and unlocked once draw commend_end processed
    int wireframe_mode;
} gpu;

enum command_types {
    COMMAND_UPLOAD,
    COMMAND_DRAW_CHUNK,
    COMMAND_DRAW_SKYBOX,
    COMMAND_SHADER_INIT,
    COMMAND_SHADER_SET_M4,
    COMMAND_SHADER_SET_TRANSFORM_MAT,
    COMMAND_SHADER_SET_FLOAT,
    COMMAND_SHADER_SET_FLOAT4,
    COMMAND_SHADER_SET_INT,
    COMMAND_SHADER_USE,
    COMMAND_SHADER_CLEANUP,
    COMMAND_SHADER_RELOAD,
    COMMAND_SCREEN_CLEAR,
    COMMAND_WIREFRAME,
    COMMAND_DRAW_START,
    COMMAND_DRAW_END,
    COMMAND_CLEANUP,
    COMMAND_COUNT
};

struct gpu_command_draw_chunk {
    uint64_t index;
};

struct gpu_command_upload {
    uint64_t chunk_index;
    chunk *c;
};

struct gpu_command_shader_mat4 {
    const char * shader_name;
    const char * uniform_name;
    mat4 value;
};

struct gpu_command_shader_float {
    const char * shader_name;
    const char * uniform_name;
    float value;
};

struct gpu_command_shader_vec4 {
    const char * shader_name;
    const char * uniform_name;
    vec4 value;
};

struct gpu_command_shader_transform_mat {
    const char * shader_name;
    const char * uniform_name;
    float value[4*4*6];
};

struct gpu_command_shader_init {
    const char * vertexPath;
    const char * fragmentPath;
    const char * name;
};

struct gpu_command_draw_skybox {
    mat4 projection;
    mat4 view;
    vec3 translate;
};


typedef struct gpu_command {
    enum command_types type;
    void * args;
} gpu_command;

struct render_thread_args {
    bool * ready;
    queue ** command_queue_handle;
    gpu ** gpu_handle;
    atlas * atlas;
    camera * cam;
    GLFWwindow ** window_handle;
};
int render_thread_init(void * thread_args);
void gpu_render_thread_stop(gpu * gpu);

gpu * gpu_init(atlas* atlas);

void gpu_set_VAO(gpu*gpu, uint64_t vao_index);

void gpu_unload(gpu* gpu, uint64_t chunk_index);

void gpu_upload(gpu* gpu, uint64_t chunk_index, chunk* c);

void gpu_draw_chunk(gpu* gpu, uint64_t index);

void gpu_draw_start(gpu* gpu);

void gpu_draw_end(gpu* gpu);

void gpu_clear_screen(gpu* gpu);

void gpu_draw_skybox(gpu* gpu, mat4 view, mat4 projection, vec3 translate);

void gpu_cycle_wireframe(gpu * gpu);

//shaders
void gpu_shader_init(gpu* gpu, const char * vertexPath, const char * fragmentPath, const char * name);
void gpu_shader_use(gpu* gpu, const char * name);
void gpu_shader_set_m4(gpu* gpu, const char * shader_name, const char * uniform_name, mat4 value);
void gpu_shader_set_transform_matrices(gpu* gpu, const char * shader_name, const char * uniform_name, mat4 *value);
void gpu_shader_set_float(gpu* gpu, const char * shader_name, const char * uniform_name, float value);
void gpu_shader_set_float4(gpu* gpu, const char * shader_name, const char * uniform_name, vec4 value);
void gpu_shader_reload(gpu* gpu);

void gpu_cleanup(gpu* gpu);