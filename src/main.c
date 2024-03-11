#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <cglm/cglm.h> 
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <window.h>
#include <shader.h>
#include <camera.h>
#include <block.h>
#include <chunk.h>
#include <world.h>
#include <atlas.h>
#include <gpu.h>
#include <frustum.h>
#ifdef __unix__
#include <pthread.h>
#endif


#define WIDTH 2300
#define HEIGHT 1200

// ToDo : Could be mat3 
static const mat4 rotation_matrix[6] = {
    {   // Top : 90 around X
        {1.f, 0.f, 0.f, 0.f},
        {0.f, 0.f, -1.f, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {0.f, 0.f, 0.f, 1.f}
    },
    {   // Bottom : -90 around X
        {1.f, 0.f, 0.f, 0.f},
        {0.f, 0.f, 1.f, 0.f},
        {0.f, -1.f, 0.f, 0.f},
        {0.f, 0.f, 0.f, 1.f}
    },
    {   // North : 180 around Y
        {-1.f, 0.f, 0.f, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {0.f, 0.f, -1.f, 0.f},
        {0.f, 0.f, 0.f, 1.f}
    },
    {   // South : nothing
        {1.f, 0.f, 0.f, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {0.f, 0.f, 1.f, 0.f},
        {0.f, 0.f, 0.f, 1.f}
    },
    {   // East : 90 around Y
        {0.f, 0.f, -1.f, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {1.f, 0.f, 0.f, 0.f},
        {0.f, 0.f, 0.f, 1.f}
    },
    {   // West : -90 around Y
        {0.f, 0.f, 1.f, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {-1.f, 0.f, 0.f, 0.f},
        {0.f, 0.f, 0.f, 1.}
    }
};

float chunk_y_offset_spawn(float chunk_time){
    const static float fall_time = 1.0f;
    float current_time = (float)glfwGetTime();
    float difference = current_time - chunk_time ;
    if (difference > fall_time) return 0.f;
    return (((1.f-powf(difference, 0.03f)) * 4000));
}

int main(int argc, char const *argv[])
{
    srand(654);
    camera * cam = camera_init();
    GLFWwindow* window = window_init(WIDTH, HEIGHT, cam);
    atlas * atlas = atlas_init(ATLAS_RESOLUTION);
    gpu * gpu = gpu_init(atlas);;
    world * w = world_init(gpu);
    player * player = player_init(cam, w);
    window_data * window_data = glfwGetWindowUserPointer(window);
    window_data->player = player;

    shader *s = shader_init("../shaders/shader.vert", "../shaders/shader.frag");
    shader *skybox = shader_init("../shaders/skybox.vert", "../shaders/skybox.frag");
    
    // Texture loading and generation
    stbi_set_flip_vertically_on_load(true);  
    unsigned int texture1;
    DEBUG_GL(glGenTextures(1, &texture1));
    DEBUG_GL(glBindTexture(GL_TEXTURE_2D, texture1)); 
    // Setting up texture filtering (when object is bigger/smaller than texture which pixel do we take from the texture image)
    DEBUG_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));	
    DEBUG_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    DEBUG_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    DEBUG_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    int width, height, nrChannels;
    const char * texture1_path ="../textures/atlas.png";
    unsigned char *data = stbi_load(texture1_path, &width, &height, &nrChannels, 0);
    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }else{
        fprintf(stderr, "Failed to load a texture : %s\n", texture1_path);
    }
    stbi_image_free(data);
    printf("textures created\n");


    shader_use(s);
    shader_set_int(s, "texture1", 0);

    DEBUG_GL(glEnable(GL_DEPTH_TEST));

    // Render loop
    unsigned int frame_count = 0;
    float last_frame_time = 0.0f;  
    float delta_time = 0.0f;
    
    DEBUG_GL(glActiveTexture(GL_TEXTURE0));
    DEBUG_GL(glBindTexture(GL_TEXTURE_2D, texture1));

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); 
    // Wireframe
    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
    printf("starting render loop\n");
    while(!window_should_close(window))
    {
        float time_frame_begin = (float)glfwGetTime();
        float current_frame_time = (float)glfwGetTime();
        delta_time = current_frame_time - last_frame_time;
        last_frame_time = current_frame_time;
        window_process_input(window, delta_time);

        // Update logic
        float time_world_update = 0.0f;
        float time_regen = 0.0f;
        float tmp_world_update = (float)glfwGetTime();
        float tmp_regen = 0.0f;
        if (world_update_position(w, cam->cameraPos[0], cam->cameraPos[2])){
            time_world_update = (float)glfwGetTime() - tmp_world_update;
            tmp_regen = (float)glfwGetTime();

        }else{
            time_world_update = (float)glfwGetTime() - tmp_world_update;
        }
        if (tmp_regen == 0.0f) tmp_regen = (float)glfwGetTime();
        world_send_update(w);
        time_regen = (float)glfwGetTime() - tmp_regen;

        float tmp_misc = (float)glfwGetTime();
        // Clearing Screen
        DEBUG_GL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
        DEBUG_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        shader_use(s);
        mat4 projection = GLM_MAT4_IDENTITY_INIT;
        float fov = 55.0f;
        glm_perspective(glm_rad(fov), (float)WIDTH / (float)HEIGHT, 0.1f, 10000.0f, projection);
        shader_set_m4(s, "view", cam->view);
        shader_set_m4(s, "projection", projection);
        shader_set_rotation_matrices(s, "rotationMatrices", (float (*)[4][4])rotation_matrix);

        // Applies frustum to world
        world_update_frustum(w, player, fov, (float)WIDTH / (float)HEIGHT, 0.1f, 10000.0f);
        float time_misc = (float)glfwGetTime() - tmp_misc; 

        // Render each chunk
        float tmp_draw = (float)glfwGetTime();
        fixray_foreach(chunk *c, w->loaded_chunks){
            if (c->in_frustum){
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            vec3 translate = {(float)(c->x) * CHUNK_X_SIZE, (float)0, (float)(c->z)*CHUNK_Z_SIZE};
            glm_translate(model, translate);
            shader_set_m4(s, "model", model);

            // Make chunk fall out of the sky
            float y_offset = chunk_y_offset_spawn(c->view_time);
            shader_set_float(s, "chunkYOffset", y_offset);
            gpu_draw(gpu, fixray_foreach_count);
            }
        }

        // Render the skybox
        {
            glDepthMask(GL_FALSE);
            shader_use(skybox);
            mat4 view_skybox = GLM_MAT4_ZERO_INIT;
            glm_mat4_copy(cam->view, view_skybox);
            view_skybox[3][3] = 1.f;
            shader_set_m4(skybox, "view", view_skybox);
            shader_set_m4(skybox, "projection", projection);
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            vec3 translate = {cam->cameraPos[0], (float)cam->cameraPos[1], (float)cam->cameraPos[2]};
            glm_translate(model, translate);
            shader_set_m4(skybox, "model", model);
            glBindVertexArray(gpu->skybox_vao);
            glBindTexture(GL_TEXTURE_CUBE_MAP, gpu->skybox_cubemap_texture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glDepthMask(GL_TRUE);
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        glFinish();
        float time_draw = (float)glfwGetTime() - tmp_draw;
        frame_count++;
        printf("Frame time : %f (regen : %f, draw %f, world %f, misc %f)\n", delta_time, time_regen, time_draw, time_world_update, time_misc);
    }

    shader_cleanup(s);
    camera_cleanup(cam);
    atlas_cleanup(atlas);
    gpu_cleanup(gpu);

    window_cleanup(window);
    return 0;
}