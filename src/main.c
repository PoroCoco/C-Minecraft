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


#define WIDTH 2300
#define HEIGHT 1200


#define DEBUG_GL(command) do { \
    command; \
    GLenum error = glGetError(); \
    if (error != GL_NO_ERROR) { \
        fprintf(stderr, "OpenGL Error at %s:%d - Code %d\n", __FILE__, __LINE__, error); \
    } \
} while(0)


static const float vertices_face_south[] = {
    0.f, 0.f,  1.f,  0.0f, 0.0f,
    1.f, 0.f,  1.f,  ATLAS_STEP, 0.0f,
    1.f,  1.f,  1.f,  ATLAS_STEP, ATLAS_STEP,
    1.f,  1.f,  1.f,  ATLAS_STEP, ATLAS_STEP,
    0.f,  1.f,  1.f,  0.0f, ATLAS_STEP,
    0.f, 0.f,  1.f,  0.0f, 0.0f,
};

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

// ToDo: Opti -> use subdata or even map it instead +  Check between static and dynamic perf for the buffer data
void regen_world_vertices(world *w, unsigned int *VAO, unsigned int *IBA, unsigned int *instances_count, atlas * atlas, bool forced){
    for (int i = 0; i < TOTAL_CHUNKS; i++){
        // Updating the IBA 
        if (w->loaded_chunks[i]->faces_dirty || w->loaded_chunks[i]->textures_dirty || w->loaded_chunks[i]->rotations_dirty || forced){
        DEBUG_GL(glBindVertexArray(VAO[i]));
        DEBUG_GL(glBindBuffer(GL_ARRAY_BUFFER, IBA[i]));
        float * instaces_offsets = chunk_get_faces_offsets(w->loaded_chunks[i], &(instances_count[i]));
        assert(MAX_FACE_IN_CHUNK > instances_count[i]); // Need to increase the max face per chunk
        DEBUG_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*instances_count[i], instaces_offsets));

        float * textures_start = chunk_get_textures(w->loaded_chunks[i], &(instances_count[i]), atlas);
        assert(MAX_FACE_IN_CHUNK > instances_count[i]); // Need to increase the max face per chunk
        DEBUG_GL(glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*MAX_FACE_IN_CHUNK*3, sizeof(float)*2*instances_count[i], textures_start));

        float * rotations_values = chunk_get_rotations_values(w->loaded_chunks[i], &(instances_count[i]));
        assert(MAX_FACE_IN_CHUNK > instances_count[i]); // Need to increase the max face per chunk
        DEBUG_GL(glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*MAX_FACE_IN_CHUNK*5, sizeof(float)*1*instances_count[i], rotations_values));
        }
    }
}

float chunk_y_offset_spawn(float chunk_time){
    const static float fall_time = 1.0f;
    float current_time = (float)glfwGetTime();
    float difference = current_time - chunk_time ;
    if (difference > fall_time) return 0.f;
    return (  ((powf(1.f-difference, 2.f)) * 1280));
}

int main(int argc, char const *argv[])
{
    srand(654);
    world * w = world_init();
    camera * cam = camera_init();
    player * player = player_init(cam, w);
    atlas * atlas = atlas_init(ATLAS_RESOLUTION);
    GLFWwindow* window = window_init(WIDTH, HEIGHT, cam, player);

    // GLAD init
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return -1;
    }    

    shader *s = shader_init("../shaders/shader.vert", "../shaders/shader.frag");
    
    // Create the VAO
    unsigned int VAO[TOTAL_CHUNKS];
    DEBUG_GL(glGenVertexArrays(TOTAL_CHUNKS, VAO));

    // Create the VBO 
    unsigned int VBO_face;
    DEBUG_GL(glGenBuffers(1, &VBO_face));

    for (size_t i = 0; i < TOTAL_CHUNKS; i++){
        DEBUG_GL(glBindVertexArray(VAO[i]));
        glBindBuffer(GL_ARRAY_BUFFER, VBO_face);
        // Upload the data for a face
        DEBUG_GL(glBufferData(GL_ARRAY_BUFFER, sizeof(float)*ATTRIBUTE_PER_VERTEX*6, vertices_face_south, GL_STATIC_DRAW)); 
        // setting its attributes
        DEBUG_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, ATTRIBUTE_PER_VERTEX * sizeof(float), (void*)0));
        DEBUG_GL(glEnableVertexAttribArray(0));
        DEBUG_GL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, ATTRIBUTE_PER_VERTEX * sizeof(float), (void*)(3 * sizeof(float))));
        DEBUG_GL(glEnableVertexAttribArray(1));
    }

    // Create the instanced array for coordinates
    const unsigned int attribute_per_instance = 6; // x,y,z; texturex,texturey; position_index;
    // Instances Buffer Attributes
    unsigned int IBA[TOTAL_CHUNKS];
    DEBUG_GL(glGenBuffers(TOTAL_CHUNKS, IBA));
    for (size_t i = 0; i < TOTAL_CHUNKS; i++)
    {
        DEBUG_GL(glBindVertexArray(VAO[i]));
        glBindBuffer(GL_ARRAY_BUFFER, IBA[i]);
        // Pre-Allocate the buffer
        DEBUG_GL(glBufferData(GL_ARRAY_BUFFER, MAX_FACE_IN_CHUNK * sizeof(float) * attribute_per_instance, NULL, GL_STATIC_DRAW)); 
        DEBUG_GL(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
        DEBUG_GL(glEnableVertexAttribArray(2));
        DEBUG_GL(glVertexAttribDivisor(2, 1));
        DEBUG_GL(glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(3 * MAX_FACE_IN_CHUNK * sizeof(float))));
        DEBUG_GL(glEnableVertexAttribArray(3));
        DEBUG_GL(glVertexAttribDivisor(3, 1));
        DEBUG_GL(glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(int), (void*)(5 * MAX_FACE_IN_CHUNK * sizeof(float))));
        DEBUG_GL(glEnableVertexAttribArray(4));
        DEBUG_GL(glVertexAttribDivisor(4, 1));
    }

    unsigned int instances_count[TOTAL_CHUNKS];

    printf("VBO binded\n");

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

    printf("starting render loop\n");
    regen_world_vertices(w, VAO, IBA, instances_count, atlas, true);
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
            regen_world_vertices(w, VAO, IBA, instances_count, atlas, true);

        }
        if (tmp_regen == 0.0f) tmp_regen = (float)glfwGetTime();
        regen_world_vertices(w, VAO, IBA, instances_count, atlas, false);
        time_regen = (float)glfwGetTime() - tmp_regen;

        // Clearing Screen
        DEBUG_GL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
        DEBUG_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        shader_use(s);
        mat4 projection = GLM_MAT4_IDENTITY_INIT;
        glm_perspective(glm_rad(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 10000.0f, projection);
        shader_set_m4(s, "view", cam->view);
        shader_set_m4(s, "projection", projection);
        shader_set_rotation_matrices(s, "rotationMatrices", (float (*)[4][4])rotation_matrix);

        // Wireframe
        // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
        float tmp_draw = (float)glfwGetTime();
        for (int i = 0 ; i < TOTAL_CHUNKS ; i++){
            DEBUG_GL(glBindVertexArray(VAO[i]));
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            vec3 translate = {(float)(w->loaded_chunks[i]->x) * CHUNK_X_SIZE, (float)0, (float)(w->loaded_chunks[i]->z)*CHUNK_Z_SIZE};
            glm_translate(model, translate);
            shader_set_m4(s, "model", model);

            // Make chunk fall out of the sky
            float y_offset = chunk_y_offset_spawn(w->loaded_chunks[i]->view_time);
            shader_set_float(s, "chunkYOffset", y_offset);
            DEBUG_GL(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instances_count[i]));
        }
        float time_draw = (float)glfwGetTime() - tmp_draw;
        
        glfwSwapBuffers(window);
        glfwPollEvents();

        frame_count++;
        printf("Frame time : %f (regen : %f, draw %f, world %f)\n", delta_time, time_regen, time_draw, time_world_update);
    }

    DEBUG_GL(glDeleteVertexArrays(TOTAL_CHUNKS, VAO));
    DEBUG_GL(glDeleteBuffers(1, &VBO_face));
    DEBUG_GL(glDeleteBuffers(TOTAL_CHUNKS, IBA));
    shader_cleanup(s);
    camera_cleanup(cam);
    atlas_cleanup(atlas);

    window_cleanup(window);
    return 0;
}