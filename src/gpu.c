#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <assert.h>
#include <gpu.h>
#include <shader.h>
#include <window.h>
#include <stb_image.h>

static const float vertices_face_south[] = {
    0.f, 0.f,  1.f,
    1.f, 0.f,  1.f,
    1.f,  1.f,  1.f,
    1.f,  1.f,  1.f,
    0.f,  1.f,  1.f,
    0.f, 0.f,  1.f
};

float skybox_vertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

const char * cubemap_faces_name[] = {
    "../textures/skybox/space/right.jpg",
    "../textures/skybox/space/left.jpg",
    "../textures/skybox/space/top.jpg",
    "../textures/skybox/space/bottom.jpg",
    "../textures/skybox/space/front.jpg",
    "../textures/skybox/space/back.jpg",
};

int64_t _bad_str_hash(char * str){
    return str[0];
}

void gpu_init_skybox(gpu * gpu){
    DEBUG_GL(glGenVertexArrays(1, &gpu->skybox_vao));
    DEBUG_GL(glGenBuffers(1, &gpu->skybox_vbo));

    DEBUG_GL(glBindVertexArray(gpu->skybox_vao));
    DEBUG_GL(glBindBuffer(GL_ARRAY_BUFFER, gpu->skybox_vbo));

    DEBUG_GL(glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*6*6, skybox_vertices, GL_STATIC_DRAW)); 
    // setting its attributes
    DEBUG_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    DEBUG_GL(glEnableVertexAttribArray(0));

}

void gpu_load_cubemap(gpu * gpu){
    glGenTextures(1, &gpu->skybox_cubemap_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, gpu->skybox_cubemap_texture);

    uint32_t cubemap_faces = 6;
    int32_t width, height, nrChannels;
    for (uint32_t i = 0; i < cubemap_faces; i++)
    {
        unsigned char *data = stbi_load(cubemap_faces_name[i], &width, &height, &nrChannels, 0);
        if (data){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }else{
            fprintf(stderr, "Cubemap tex failed to load at path: %s\n", cubemap_faces_name[i]);
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}


gpu * gpu_init(atlas * atlas){
    gpu *gpu = malloc(sizeof(*gpu));
    gpu->atlas = atlas;
    gpu->shaders = htb_init(256);

    // GLAD init
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return NULL;
    }    

    gpu->command_queue = queue_init(128); //Todo : Maybe use a dynamic one instead

    assert(mtx_init(&gpu->mutex, mtx_plain) == thrd_success);
    assert(mtx_init(&gpu->draw_mutex, mtx_plain) == thrd_success);

    printf("Starting OpenGL objects creation\n");
    // OpenGL Objects
    // Create the VAOs
    DEBUG_GL(glGenVertexArrays(TOTAL_CHUNKS, gpu->VAO));

    // Create the VBO 
    DEBUG_GL(glGenBuffers(1, &gpu->VBO_face));

    for (size_t i = 0; i < TOTAL_CHUNKS; i++){
        DEBUG_GL(glBindVertexArray(gpu->VAO[i]));
        glBindBuffer(GL_ARRAY_BUFFER, gpu->VBO_face);
        // Upload the data for a face
        DEBUG_GL(glBufferData(GL_ARRAY_BUFFER, sizeof(float)*ATTRIBUTE_PER_VERTEX*6, vertices_face_south, GL_STATIC_DRAW)); 
        // setting its attributes
        DEBUG_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, ATTRIBUTE_PER_VERTEX * sizeof(float), (void*)0));
        DEBUG_GL(glEnableVertexAttribArray(0));
    }

    // Create the Instances Buffer Attributes
    DEBUG_GL(glGenBuffers(TOTAL_CHUNKS, gpu->IBA));
    for (size_t i = 0; i < TOTAL_CHUNKS; i++)
    {
        DEBUG_GL(glBindVertexArray(gpu->VAO[i]));
        glBindBuffer(GL_ARRAY_BUFFER, gpu->IBA[i]);
        // Pre-Allocate the buffer
        DEBUG_GL(glBufferData(GL_ARRAY_BUFFER, MAX_FACE_IN_CHUNK * sizeof(int32_t) * PACKED_4BYTES_COUNT, NULL, GL_STATIC_DRAW)); 
        // Packed Data
        DEBUG_GL(glVertexAttribIPointer(1, PACKED_4BYTES_COUNT, GL_INT, PACKED_4BYTES_COUNT * sizeof(int32_t), (void*)0));
        DEBUG_GL(glEnableVertexAttribArray(1));
        DEBUG_GL(glVertexAttribDivisor(1, 1));
    }

    // Texture loading and generation
    stbi_set_flip_vertically_on_load(true);  
    unsigned int atlas_texture;
    DEBUG_GL(glGenTextures(1, &atlas_texture));
    DEBUG_GL(glBindTexture(GL_TEXTURE_2D_ARRAY, atlas_texture)); 
    // Setting up texture filtering (when object is bigger/smaller than texture which pixel do we take from the texture image)
    DEBUG_GL(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT));	
    DEBUG_GL(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT));
    DEBUG_GL(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    DEBUG_GL(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    int width, height, nrChannels;
    const char * atlas_texture_path ="../textures/atlas.png";
    unsigned char *data = stbi_load(atlas_texture_path, &width, &height, &nrChannels, 0);
    int tile_x_resolution = ATLAS_TILE_WIDTH;
    int tile_y_resolution = ATLAS_TILE_HEIGHT;
    DEBUG_GL(glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, width/tile_x_resolution, height/tile_y_resolution, ATLAS_TOTAL_TILES, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
    if (data){
        // Upload each tile from the atlas by getting the tile data from the atlas memory
        for (size_t tile = 0; tile < tile_x_resolution*tile_y_resolution; tile++){
            unsigned char *tile_data = malloc(sizeof(*tile_data) * tile_x_resolution*tile_y_resolution*nrChannels);

            for (size_t tile_row = 0; tile_row < tile_y_resolution; tile_row++){
                memcpy(tile_data + tile_row*tile_x_resolution*nrChannels, data + (tile/ATLAS_TILE_PER_COL * width*tile_y_resolution*nrChannels) + ((tile%ATLAS_TILE_PER_ROW) * tile_x_resolution*nrChannels) + (tile_row*width*nrChannels), tile_x_resolution*nrChannels);
            }
            
            DEBUG_GL(glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                            0, 0, tile,
                            tile_x_resolution, tile_y_resolution, 1,
                            GL_RGB, GL_UNSIGNED_BYTE, tile_data));
            free(tile_data);
        }
        
    }else{
        fprintf(stderr, "Failed to load a texture : %s\n", atlas_texture_path);
    }
    DEBUG_GL(glGenerateMipmap(GL_TEXTURE_2D_ARRAY));
    stbi_image_free(data);
    printf("textures created\n");

    DEBUG_GL(glActiveTexture(GL_TEXTURE0));

    // Setting some GL options
    DEBUG_GL(glEnable(GL_DEPTH_TEST));
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); 
    // Wireframe
    gpu->wireframe_mode = GL_FILL;
    DEBUG_GL(glPolygonMode(GL_FRONT_AND_BACK, gpu->wireframe_mode));
    DEBUG_GL(glLineWidth(5.f));

    // skybox init
    gpu_load_cubemap(gpu);
    gpu_init_skybox(gpu);

    printf("OpenGL Objects creation successful\n");
    return gpu;
}


void _gpu_create_command(gpu * gpu, enum command_type type, void * args){
    gpu_command *com = malloc(sizeof(*com));
    assert(com);
    com->type = type;
    com->args = args;
    queue_enqueue(gpu->command_queue, (void *) com);
}

void gpu_set_VAO(gpu* gpu, uint64_t vao_index){
    DEBUG_GL(glBindVertexArray(gpu->VAO[vao_index]));
}

void gpu_unload(gpu* gpu, uint64_t chunk_index){
    // Do nothing, you just need to upload on top of this now old data
}


void gpu_upload(gpu* gpu, uint64_t chunk_index, chunk *c){
    struct gpu_command_upload * args = malloc(sizeof(*args));
    args->c = c;
    args->chunk_index = chunk_index;
    _gpu_create_command(gpu, COMMAND_UPLOAD, (void*) args);
}

// ToDo: Opti -> use subdata or even map it instead +  Check between static and dynamic perf for the buffer data
void _gpu_upload(gpu* gpu, uint64_t chunk_index, chunk *c){
    DEBUG_GL(glBindVertexArray(gpu->VAO[chunk_index]));
    DEBUG_GL(glBindBuffer(GL_ARRAY_BUFFER, gpu->IBA[chunk_index]));
    
    gpu->instances_count[chunk_index] = chunk_get_mesh(c, gpu->atlas);
    DEBUG_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(int32_t)*PACKED_4BYTES_COUNT*gpu->instances_count[chunk_index], c->faces_packed_data));
}

void gpu_draw(gpu* gpu, uint64_t index){
    struct gpu_command_draw_chunk * args = malloc(sizeof(*args));
    args->index = index;
    _gpu_create_command(gpu, COMMAND_DRAW_CHUNK, (void*) args);
}

void _gpu_draw(gpu* gpu, struct gpu_command_draw_chunk *args){
    gpu_set_VAO(gpu, args->index);
    DEBUG_GL(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, gpu->instances_count[args->index]));
}


void _gpu_clear_screen(gpu* gpu){
    // Clearing Screen
    DEBUG_GL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
    DEBUG_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void gpu_clear_screen(gpu* gpu){
    _gpu_create_command(gpu, COMMAND_SCREEN_CLEAR, NULL);
}

void gpu_draw_end(gpu* gpu){
    _gpu_create_command(gpu, COMMAND_DRAW_END, NULL);
    mtx_lock(&gpu->draw_mutex);
    mtx_unlock(&gpu->draw_mutex);
}

void gpu_draw_start(gpu* gpu){
    _gpu_create_command(gpu, COMMAND_DRAW_START, NULL);
}

void _gpu_shader_cleanup(gpu* gpu, struct gpu_command_shader_init * args){
    shader *s = htb_get(gpu->shaders, _bad_str_hash(args->name));
    htb_remove(gpu->shaders, _bad_str_hash(args->name));
    shader_cleanup(s);
}

void gpu_shader_cleanup(gpu* gpu, const char * name){
    struct gpu_command_shader_init * args = malloc(sizeof(*args));
    args->name = name;
    args->fragmentPath = NULL;
    args->vertexPath = NULL;
    _gpu_shader_cleanup(gpu, (void*) args);
}

void gpu_shader_init(gpu* gpu, const char * vertexPath, const char * fragmentPath, const char * name){
    struct gpu_command_shader_init * args = malloc(sizeof(*args));
    args->fragmentPath = fragmentPath;
    args->vertexPath = vertexPath;
    args->name = name;
    _gpu_create_command(gpu, COMMAND_SHADER_INIT, (void*) args);
}

void _gpu_shader_init(gpu* gpu, struct gpu_command_shader_init * args){
    shader * s = shader_init(args->vertexPath, args->fragmentPath);
    htb_add(gpu->shaders, _bad_str_hash(args->name), s);
}

void gpu_shader_reload(gpu* gpu){
    _gpu_create_command(gpu, COMMAND_SHADER_RELOAD, NULL);
}

void _gpu_shader_reload(gpu* gpu){
    // Remove previous shaders
    struct gpu_command_shader_init * args = malloc(sizeof(*args));
    if (htb_exist(gpu->shaders, _bad_str_hash("chunk"))){
        args->name = "chunk";
        _gpu_shader_cleanup(gpu, args);
    }
    if (htb_exist(gpu->shaders, _bad_str_hash("skybox"))){
        args->name = "skybox";
        _gpu_shader_cleanup(gpu, args);
    }
    args->name = "chunk";
    args->vertexPath = "../shaders/shader.vert";
    args->fragmentPath = "../shaders/shader.frag";
    _gpu_shader_init(gpu, args);

    args->name = "skybox";
    args->vertexPath = "../shaders/skybox.vert";
    args->fragmentPath = "../shaders/skybox.frag";
    _gpu_shader_init(gpu, args);
}


void gpu_shader_use(gpu* gpu, const char * name){
    struct gpu_command_shader_init * args = malloc(sizeof(*args));
    args->name = name;
    _gpu_create_command(gpu, COMMAND_SHADER_USE, (void*) args);
}

void _gpu_shader_use(gpu* gpu, struct gpu_command_shader_init * args){
    shader *s = htb_get(gpu->shaders, _bad_str_hash(args->name));
    shader_use(s);
}

void gpu_draw_skybox(gpu* gpu, mat4 view, mat4 projection, vec3 translate){
    struct gpu_command_draw_skybox * args = malloc(sizeof(*args));
    glm_mat4_copy(view, args->view);
    glm_mat4_copy(projection, args->projection);
    glm_vec3_copy(translate, args->translate);
    _gpu_create_command(gpu, COMMAND_DRAW_SKYBOX, (void*) args);
}

void _gpu_draw_skybox(gpu* gpu, struct gpu_command_draw_skybox * args){
    shader * skybox = htb_get(gpu->shaders, _bad_str_hash("skybox"));
    glDepthMask(GL_FALSE);
    shader_use(skybox);
    mat4 view_skybox = GLM_MAT4_ZERO_INIT;
    glm_mat4_copy(args->view, view_skybox);
    view_skybox[3][3] = 1.f;
    shader_set_m4(skybox, "view", view_skybox);
    shader_set_m4(skybox, "projection", args->projection);
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_translate(model, args->translate);
    shader_set_m4(skybox, "model", model);
    glBindVertexArray(gpu->skybox_vao);
    glBindTexture(GL_TEXTURE_CUBE_MAP, gpu->skybox_cubemap_texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
}

void gpu_shader_set_m4(gpu* gpu, const char * shader_name, const char * uniform_name, mat4 value){
    struct gpu_command_shader_mat4 * args = malloc(sizeof(*args));
    glm_mat4_copy(value, args->value);
    args->shader_name = shader_name;
    args->uniform_name = uniform_name;
    _gpu_create_command(gpu, COMMAND_SHADER_SET_M4, (void*) args);
}

void _gpu_shader_set_m4(gpu* gpu, struct gpu_command_shader_mat4 * args){
    shader *s = htb_get(gpu->shaders, _bad_str_hash(args->shader_name));
    shader_set_m4(s, args->uniform_name, args->value);
}

void gpu_shader_set_float(gpu* gpu, const char * shader_name, const char * uniform_name, float value){
    struct gpu_command_shader_float * args = malloc(sizeof(*args));
    args->value = value;
    args->shader_name = shader_name;
    args->uniform_name = uniform_name;
    _gpu_create_command(gpu, COMMAND_SHADER_SET_FLOAT, (void*) args);
}

void _gpu_shader_set_float(gpu* gpu, struct gpu_command_shader_float * args){
    shader *s = htb_get(gpu->shaders, _bad_str_hash(args->shader_name));
    shader_set_float(s, args->uniform_name, args->value);
}

void gpu_shader_set_transform_matrices(gpu* gpu, const char * shader_name, const char * uniform_name, mat4* value){
    struct gpu_command_shader_transform_mat * args = malloc(sizeof(*args));
    memcpy(args->value, value, sizeof(float)*4*4*6); //6 4*4 matrices
    args->shader_name = shader_name;
    args->uniform_name = uniform_name;
    _gpu_create_command(gpu, COMMAND_SHADER_SET_TRANSFORM_MAT, (void*) args);
}

void _gpu_shader_set_transform_matrices(gpu* gpu, struct gpu_command_shader_transform_mat * args){
    shader *s = htb_get(gpu->shaders, _bad_str_hash(args->shader_name));
    shader_set_transform_matrices(s, args->uniform_name, (mat4 *) args->value);
}

void gpu_cycle_wireframe(gpu * gpu){
    _gpu_create_command(gpu, COMMAND_WIREFRAME, NULL);
}

void _gpu_cycle_wireframe(gpu * gpu){
    if(gpu->wireframe_mode == GL_LINE){
        gpu->wireframe_mode = GL_FILL;
    }else if(gpu->wireframe_mode == GL_FILL){
        gpu->wireframe_mode = GL_POINT;
    }else{
        gpu->wireframe_mode = GL_LINE;
    }
    glPolygonMode( GL_FRONT_AND_BACK, gpu->wireframe_mode);
}


void gpu_render_thread_stop(gpu * gpu){
    _gpu_create_command(gpu, COMMAND_CLEANUP, NULL);
}

void gpu_cleanup(gpu* gpu){
    DEBUG_GL(glDeleteVertexArrays(TOTAL_CHUNKS, gpu->VAO));
    DEBUG_GL(glDeleteBuffers(1, &gpu->VBO_face));
    DEBUG_GL(glDeleteBuffers(TOTAL_CHUNKS, gpu->IBA));
    mtx_destroy(&gpu->mutex);
    mtx_destroy(&gpu->draw_mutex);
    queue_cleanup(gpu->command_queue);
    htb_cleanup(gpu->shaders, free);
    free(gpu);
}

int render_thread_init(void * thread_args){
    struct render_thread_args * th_args = thread_args; 

    *th_args->window_handle = window_init(WIDTH, HEIGHT, th_args->cam);;
    gpu * gpu = gpu_init(th_args->atlas);
    *th_args->command_queue_handle = gpu->command_queue;
    *th_args->gpu_handle = gpu;
    *th_args->ready = true;

    // loop on the command queue
    bool running = true;
    while(running){
        if(!queue_is_empty(gpu->command_queue)){
            gpu_command *command = queue_dequeue(gpu->command_queue);
            switch (command->type)
            {
            case COMMAND_UPLOAD:
            {
                struct gpu_command_upload * args = command->args;
                _gpu_upload(gpu, args->chunk_index, args->c);
                break; 
            }    
            case COMMAND_DRAW_CHUNK:
            {
                struct gpu_command_draw_chunk * args = command->args;
                _gpu_draw(gpu, args);
                break; 
            }    
            case COMMAND_SCREEN_CLEAR:
            {
                _gpu_clear_screen(gpu);
                break; 
            }    
            case COMMAND_DRAW_END:
            {
                glfwSwapBuffers(*th_args->window_handle);
                glfwPollEvents();
                mtx_unlock(&gpu->draw_mutex);
                break; 
            }    
            case COMMAND_DRAW_START:
            {
                mtx_lock(&gpu->draw_mutex);
                break; 
            }    
            case COMMAND_SHADER_INIT:
            {
                struct gpu_command_shader_init * args = command->args;
                _gpu_shader_init(gpu, args);
                break; 
            }    
            case COMMAND_SHADER_USE:
            {
                struct gpu_command_shader_init * args = command->args;
                _gpu_shader_use(gpu, args);
                break; 
            }    
            case COMMAND_DRAW_SKYBOX:
            {
                struct gpu_command_draw_skybox * args = command->args;
                _gpu_draw_skybox(gpu, args);
                break; 
            }    
            case COMMAND_SHADER_SET_M4:
            {
                struct gpu_command_shader_mat4 * args = command->args;
                _gpu_shader_set_m4(gpu, args);
                break; 
            }    
            case COMMAND_SHADER_SET_TRANSFORM_MAT:
            {
                struct gpu_command_shader_transform_mat * args = command->args;
                _gpu_shader_set_transform_matrices(gpu, args);
                break; 
            }    
            case COMMAND_SHADER_SET_FLOAT:
            {
                struct gpu_command_shader_float * args = command->args;
                _gpu_shader_set_float(gpu, args);
                break; 
            }
            case COMMAND_SHADER_CLEANUP:
            {
                struct gpu_command_shader_init * args = command->args;
                _gpu_shader_cleanup(gpu, args);
                break; 
            }   
            case COMMAND_SHADER_RELOAD:
            {
                _gpu_shader_reload(gpu);
                break; 
            }   
            case COMMAND_WIREFRAME:
            {
                _gpu_cycle_wireframe(gpu);
                break;
            }
            case COMMAND_CLEANUP:
            {
                running = false;
                break;
            }
            default:
                break;
            }
            free(command->args);
            free(command);
        }
    }

    return 0;
}
