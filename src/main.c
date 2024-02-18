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


#define WIDTH 1600
#define HEIGHT 1200


int main(int argc, char const *argv[])
{
    camera * cam = camera_init();
    GLFWwindow* window = window_init(WIDTH, HEIGHT, cam);

    // GLAD init
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return -1;
    }    

    shader *s = shader_init("../shaders/shader.vert", "../shaders/shader.frag");
    
    // // Rectangle
    // float vertices[] = {
    //     // positions          // colors           // texture coords
    //     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
    //     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    //     -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    //     -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
    // };
    // Cube
    static const float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    

    // Indices for rectangle
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    
    // Setting the vertex attributes and VBO inside a VAO (Vertex Array object)
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);  
    glBindVertexArray(VAO);
    
    // Create the Vertex Buffer Object
    unsigned int VBO;
    glGenBuffers(1, &VBO);  
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Sends the data
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    chunk * c = chunk_init(0,0);
    int vertices_count;
    float * chunk_vertices = chunk_generate_vertices(c, &vertices_count);
    printf("main vertice count = %d\n main size %zu ", vertices_count, sizeof(float) * vertices_count * 5);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_count * 5, chunk_vertices, GL_STATIC_DRAW);
    free(chunk_vertices);
    chunk_cleanup(c);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
    // glEnableVertexAttribArray(1);
    // texture coord attribute 
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1); 

    // Create the Element Object Buffer (the indices bufffer)
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // Sends the data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

    // Unbinds the VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Do not unbind the EBO
    // -> remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    // Unbinds the VAO
    glBindVertexArray(0);


    // Texture loading and generation
    stbi_set_flip_vertically_on_load(true);  
    unsigned int texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1); 
    // Setting up texture filtering (when object is bigger/smaller than texture which pixel do we take from the texture image)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    const char * texture1_path ="../textures/container.jpg";
    unsigned char *data = stbi_load(texture1_path, &width, &height, &nrChannels, 0);
    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }else{
        fprintf(stderr, "Failed to load a texture : %s\n", texture1_path);
    }
    stbi_image_free(data);

    unsigned int texture2;
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    const char * texture2_path = "../textures/awesomeface.png";
    data = stbi_load(texture2_path, &width, &height, &nrChannels, 0);
    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }else{
        fprintf(stderr, "Failed to load a texture : %s\n", texture2_path);
    }
    stbi_image_free(data);

    shader_use(s);
    shader_set_int(s, "texture1", 0);
    shader_set_int(s, "texture2", 1);



    glEnable(GL_DEPTH_TEST);

    // Camera 
    // vec3 cameraPos = {0.0f, 0.0f, 3.0f};
    // vec3 cameraTarget = {0.0f, 0.0f, 0.0f}; // The center of the world
    // vec3 cameraDirection = glm_vec3_norm(cameraPos - cameraTarget); // Vector substraction yields the vector between the two. Careful, the direction is actually from the origin to the camera
    // vec3 up = {0.0f, 1.0f, 0.0f}; 
    // vec3 cameraRight;
    // glm_vec3_cross(up, cameraDirection, cameraRight);
    // glm_vec3_norm(cameraRight);
    // vec3 cameraUp;
    // glm_vec3_cross(cameraDirection, cameraRight, cameraUp);
    // is equivalent to 
    // mat4 view;
    // glm_lookat((vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f}, view);

    // Render loop
    unsigned int frame_count = 0;
    float last_frame_time = 0.0f;  
    float delta_time = 0.0f;
    while(!window_should_close(window))
    {
        float current_frame_time = (float)glfwGetTime();
        delta_time = current_frame_time - last_frame_time;
        last_frame_time = current_frame_time;
        window_process_input(window, delta_time);

        // Rendering
        // Clearing Screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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


        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        // Wireframe
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        for (size_t x = 0; x < 1; x++){
            for (size_t z = 0; z < 1; z++){
                mat4 model = GLM_MAT4_IDENTITY_INIT;
                glm_translate(model, (vec3){(float)x*16, 0.0, (float)z*16});
                shader_set_m4(s, "model", model);
                glDrawArrays(GL_TRIANGLES, 0, vertices_count);
            }
        }
        
        
        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        frame_count++;
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    shader_cleanup(s);
    camera_cleanup(cam);

    window_cleanup(window);
    return 0;
}