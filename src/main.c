#include <stdio.h>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <window.h>
#include <shader.h>
#include <math.h>

int main(int argc, char const *argv[])
{
    GLFWwindow* window = window_init();

    // GLAD init
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return -1;
    }    

    shader *s = shader_init("../shaders/shader.vert", "../shaders/shader.frag");
    
    float vertices[] = {
        // positions         // colors
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
        0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top 
    };  
    unsigned int indices[] = { 
        0, 1, 2,   // first triangle
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
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


    // Render loop
    unsigned int frame_count = 0;
    while(!window_should_close(window))
    {
        window_process_input(window);

        // Rendering
        // Clearing Screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Triangle render

        shader_use(s);
        // update the uniform color
        float timeValue = (float)glfwGetTime();
        float greenValue = sinf(timeValue) / 2.0f + 0.5f;
        // shader_set_float4(s, "ourColor", 0.0f, greenValue, 0.0f, 1.0f);
        shader_set_float4(s, "ourMove", (cosf(timeValue) / 2.0f), (sinf(timeValue) / 2.0f), 0.0f, 1.0f);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        frame_count++;
    }


    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    shader_cleanup(s);

    window_cleanup();
    return 0;
}
