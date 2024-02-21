#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <math.h>
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

void regen_world_vertices(world *w, unsigned int *VAO, unsigned int *EBO, unsigned int *elements_count, atlas * atlas){
    for (int i = 0; i < TOTAL_CHUNKS; i++){
        DEBUG_GL(glBindVertexArray(VAO[i]));
        DEBUG_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]));
        DEBUG_GL(glDeleteBuffers(1, &EBO[i]));
        DEBUG_GL(glGenBuffers(1, &EBO[i]));
        DEBUG_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]));
        unsigned int * elements = chunk_get_elements(w->loaded_chunks[i], &(elements_count[i]), atlas);
        DEBUG_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*elements) * elements_count[i], elements, GL_STATIC_DRAW));
    }
}

int main(int argc, char const *argv[])
{
    srand(654);
    world * w = world_init();
    camera * cam = camera_init();
    player * player = player_init(cam, w);
    atlas * atlas = atlas_init(256);
    GLFWwindow* window = window_init(WIDTH, HEIGHT, cam, player);

    // GLAD init
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return -1;
    }    

    shader *s = shader_init("../shaders/shader.vert", "../shaders/shader.frag");
    
    // Create the VAOs
    unsigned int VAO[TOTAL_CHUNKS];
    DEBUG_GL(glGenVertexArrays(TOTAL_CHUNKS, VAO));

    // Create the unique VBO and fill it
    unsigned int VBO;
    DEBUG_GL(glGenBuffers(1, &VBO));
    DEBUG_GL(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    unsigned int vertice_count;
    float * chunk_static_vertices = chunk_generate_static_mesh(atlas, &vertice_count);
    printf("count %d\n", vertice_count);
    DEBUG_GL(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertice_count * ATTRIBUTE_PER_VERTEX, chunk_static_vertices, GL_STATIC_DRAW));
    free(chunk_static_vertices);

    // Bind the same VBO for each
    for (size_t i = 0; i < TOTAL_CHUNKS; i++){
        DEBUG_GL(glBindVertexArray(VAO[i]));
        DEBUG_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, ATTRIBUTE_PER_VERTEX * sizeof(float), (void*)0));
        DEBUG_GL(glEnableVertexAttribArray(0));
        DEBUG_GL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, ATTRIBUTE_PER_VERTEX * sizeof(float), (void*)(3 * sizeof(float))));
        DEBUG_GL(glEnableVertexAttribArray(1)); 
    }
    printf("VBO binded\n");

    // Each chunk return its EBO from its block data
    // Each chunk return its texturesCoord from its block data

    // // Setting the vertex attributes and VBO inside a VAO (Vertex Array object)
    // unsigned int VAO[TOTAL_CHUNKS];
    // glGenVertexArrays(TOTAL_CHUNKS, VAO);  
    
    // // Create the Vertex Buffer Object
    // unsigned int VBO[TOTAL_CHUNKS];
    // glGenBuffers(TOTAL_CHUNKS, VBO);

    // // Sends the data
    // // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // int vertices_count[TOTAL_CHUNKS];

    // for (int i = 0; i < TOTAL_CHUNKS; i++){
    //     glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
    //     float * chunk_vertices = chunk_get_vertices(w->loaded_chunks[i], &vertices_count[i], atlas);
    //     glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_count[i] * 5, chunk_vertices, GL_STATIC_DRAW);
    //     glBindVertexArray(VAO[i]);
    //     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    //     glEnableVertexAttribArray(0);
    //     glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    //     glEnableVertexAttribArray(1); 
    // }
    
    // position attribute
    // color attribute
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
    // glEnableVertexAttribArray(1);
    // texture coord attribute 

    // Create the Element Object Buffer (the indices bufffer)
    unsigned int EBO[TOTAL_CHUNKS];
    DEBUG_GL(glGenBuffers(TOTAL_CHUNKS, EBO));
    unsigned int elements_count[TOTAL_CHUNKS];
    // Sends the data
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

    // Unbinds the VBO
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Do not unbind the EBO
    // -> remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    // Unbinds the VAO
    // glBindVertexArray(0);
    printf("EBO created\n");


    // Texture loading and generation
    stbi_set_flip_vertically_on_load(true);  
    unsigned int texture1;
    DEBUG_GL(glGenTextures(1, &texture1));
    DEBUG_GL(glBindTexture(GL_TEXTURE_2D, texture1)); 
    // Setting up texture filtering (when object is bigger/smaller than texture which pixel do we take from the texture image)
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

    // unsigned int texture2;
    // glGenTextures(1, &texture2);
    // glBindTexture(GL_TEXTURE_2D, texture2); 
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // const char * texture2_path = "../textures/awesomeface.png";
    // data = stbi_load(texture2_path, &width, &height, &nrChannels, 0);
    // if (data){
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //     glGenerateMipmap(GL_TEXTURE_2D);
    // }else{
    //     fprintf(stderr, "Failed to load a texture : %s\n", texture2_path);
    // }
    // stbi_image_free(data);

    shader_use(s);
    shader_set_int(s, "texture1", 0);
    // shader_set_int(s, "texture2", 1);

    DEBUG_GL(glEnable(GL_DEPTH_TEST));

    // Render loop
    unsigned int frame_count = 0;
    float last_frame_time = 0.0f;  
    float delta_time = 0.0f;
    
    DEBUG_GL(glActiveTexture(GL_TEXTURE0));
    DEBUG_GL(glBindTexture(GL_TEXTURE_2D, texture1));
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
        world_update_position(w, cam->cameraPos[0], cam->cameraPos[2]);
        time_world_update = (float)glfwGetTime() - tmp_world_update;

        float tmp_regen = (float)glfwGetTime();
        regen_world_vertices(w, VAO, EBO, elements_count, atlas);
        time_regen = (float)glfwGetTime() - tmp_regen;


        // Rendering
        // Clearing Screen
        DEBUG_GL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
        DEBUG_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        // Triangle render

        shader_use(s);
        // update the uniform color
        float timeValue = (float)glfwGetTime();
        float greenValue = sinf(timeValue) / 2.0f + 0.5f;
        // shader_set_float4(s, "ourColor", 0.0f, greenValue, 0.0f, 1.0f);
        // shader_set_float4(s, "ourMove", (cosf(timeValue) / 2.0f), (sinf(timeValue) / 2.0f), 0.0f, 1.0f);


        mat4 projection = GLM_MAT4_IDENTITY_INIT;
        glm_perspective(glm_rad(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 10000.0f, projection);
        shader_set_m4(s, "view", cam->view);
        shader_set_m4(s, "projection", projection);


        // glm_rotate(trans, glm_rad(90.0f), (vec3){0.0f, 0.0f, 1.0f});
        // glm_scale(trans, (vec3){0.75f, 0.75f, 0.75f}); 
        // shader_set_m4(s, "transform", trans);

        // Wireframe
        // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
        float tmp_draw = (float)glfwGetTime();
        for (int i = 0 ; i < TOTAL_CHUNKS ; i++){
            DEBUG_GL(glBindVertexArray(VAO[i]));
            DEBUG_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]));
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            vec3 translate = {(float)(w->loaded_chunks[i]->x) * CHUNK_X_SIZE, (float)0, (float)(w->loaded_chunks[i]->z)*CHUNK_Z_SIZE};
            glm_translate(model, translate);
            shader_set_m4(s, "model", model);
            // glDrawArrays(GL_TRIANGLES, 0, vertice_count);
            DEBUG_GL(glDrawElements(GL_TRIANGLES, elements_count[i], GL_UNSIGNED_INT, 0));
        }
        float time_draw = (float)glfwGetTime() - tmp_draw;
        
        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        DEBUG_GL(glBindVertexArray(0));

        glfwSwapBuffers(window);
        glfwPollEvents();

        frame_count++;
        // printf("Frame time : %f (regen : %f, draw %f, world %f)\n", delta_time, time_regen, time_draw, time_world_update);
    }

    DEBUG_GL(glDeleteVertexArrays(TOTAL_CHUNKS, VAO));
    DEBUG_GL(glDeleteBuffers(1, &VBO));
    DEBUG_GL(glDeleteBuffers(TOTAL_CHUNKS, EBO));
    shader_cleanup(s);
    camera_cleanup(cam);
    atlas_cleanup(atlas);

    window_cleanup(window);
    return 0;
}