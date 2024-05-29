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
#include <timing.h>
#include <pthread.h>


// These are defined as row major but read as column major ie the translation vector is the last row not the last column
static const mat4 transform_matrices[6] = {
    {   // Top : 90 around X
        {1.f, 0.f, 0.f, 0.f},
        {0.f, 0.f, -1.f, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {0.f, 0.f, 1.f, 1.f}
    },
    {   // Bottom : -90 around X
        {1.f, 0.f, 0.f, 0.f},
        {0.f, 0.f, 1.f, 0.f},
        {0.f, -1.f, 0.f, 0.f},
        {0.f, 1.f, 0.f, 1.f}
    },
    {   // North : 180 around Y
        {-1.f, 0.f, 0.f, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {0.f, 0.f, -1.f, 0.f},
        {1.f, 0.f, 1.f, 1.f}
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
        {0.f, 0.f, 1.f, 1.f}
    },
    {   // West : -90 around Y
        {0.f, 0.f, 1.f, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {-1.f, 0.f, 0.f, 0.f},
        {1.f, 0.f, 0.f, 1.}
    }
};

float chunk_y_offset_spawn(float chunk_time){
    static const float fall_time = 1.0f;
    float current_time = (float)glfwGetTime();
    float difference = current_time - chunk_time ;
    if (difference > fall_time) return 0.f;
    return (((1.f-powf(difference, 0.03f)) * 4000));
}

int main(int argc, char const *argv[])
{
    srand(654);
    camera * cam = camera_init();
    atlas * atlas = atlas_init();
    gpu * gpu;
    queue * command_queue_handle;
    GLFWwindow* window;
    volatile bool render_thread_ready = false;
    pthread_t render_thread;
    struct render_thread_args args = {
        .atlas = atlas,
        .command_queue_handle = &command_queue_handle,
        .gpu_handle = &gpu,
        .ready = &render_thread_ready,
        .cam = cam,
        .window_handle = &window
    };
    int render_thread_id = pthread_create(&render_thread, NULL, render_thread_init, (void*) &args);
    while (!render_thread_ready){
        
    }
    gpu_shader_reload(gpu);

    world * w = world_init(gpu);
    player * player = player_init(cam, w);
    window_data * window_data = glfwGetWindowUserPointer(window);
    window_data->player = player;
    window_data->gpu = gpu;
    
    // Render loop
    unsigned int frame_count = 0;
    float last_frame_time = 0.0f;  
    float delta_time = 0.0f;

    printf("starting render loop\n");
    while(!window_should_close(window))
    {
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
        gpu_clear_screen(gpu);
        gpu_draw_start(gpu);

        gpu_shader_use(gpu, "chunk");
        mat4 projection = GLM_MAT4_IDENTITY_INIT;
        float fov = 55.0f;
        glm_perspective(glm_rad(fov), (float)WIDTH / (float)HEIGHT, 0.1f, 10000.0f, projection);
        gpu_shader_set_m4(gpu, "chunk", "view", cam->view);
        gpu_shader_set_m4(gpu, "chunk", "projection", projection);
        gpu_shader_set_transform_matrices(gpu, "chunk", "transfromMatrices", (float (*)[4][4])transform_matrices);

        // Applies frustum to world
        world_update_frustum(w, player, fov, (float)WIDTH / (float)HEIGHT, 0.1f, 10000.0f);
        // gpu_shader_reload(gpu); // Allow realtime shader modification
        float time_misc = (float)glfwGetTime() - tmp_misc; 

        // Render each chunk
        float tmp_draw = (float)glfwGetTime();
        fixray_foreach(chunk *c, w->loaded_chunks){
            if (c->ready && c->in_frustum){
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            vec3 translate = {(float)(c->x) * CHUNK_X_SIZE, (float)0, (float)(c->z)*CHUNK_Z_SIZE};
            glm_translate(model, translate);
            gpu_shader_set_m4(gpu, "chunk", "model", model);

            // Make chunk fall out of the sky | Todo : current time should be a parameter to only fetch the time once before this loop.
            // float y_offset = chunk_y_offset_spawn(c->timestap_generation);
            // gpu_shader_set_float(gpu, "chunk", "chunkYOffset", y_offset);

            gpu_draw_chunk(gpu, fixray_foreach_count);

            }
        }

        // Render the skybox
        vec3 translate = {cam->cameraPos[0], (float)cam->cameraPos[1], (float)cam->cameraPos[2]};
        gpu_draw_skybox(gpu, cam->view, projection, translate);
        
        gpu_draw_end(gpu);
        // glFinish();
        float time_draw = (float)glfwGetTime() - tmp_draw;
        frame_count++;
        // printf("Frame time : %f (regen : %f, draw %f, world %f, misc %f)\n", delta_time, time_regen, time_draw, time_world_update, time_misc);
    }

    gpu_render_thread_stop(gpu);
    // shader_cleanup(s);
    // shader_cleanup(skybox);
    camera_cleanup(cam);
    atlas_cleanup(atlas);
    player_cleanup(player);
    world_cleanup(w);

    window_cleanup(window);
    return 0;
}