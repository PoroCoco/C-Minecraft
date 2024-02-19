#include <glad/glad.h> 
#include <window.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void window_mouse_callback(GLFWwindow* window, double x_pos, double y_pos){
    window_data * w = glfwGetWindowUserPointer(window);
    if (w->mouse_first){
        w->mouse_last_x = x_pos;
        w->mouse_last_y = y_pos;
        w->mouse_first = false;
    }

    double x_offset = x_pos - w->mouse_last_x;
    double y_offset = w->mouse_last_y - y_pos;
    w->mouse_last_x = x_pos;
    w->mouse_last_y = y_pos;

    double sensitivity = 0.1f;
    x_offset *= sensitivity;
    y_offset *= sensitivity;

    camera_mouse_callback(w->cam, x_offset, y_offset);
}

void window_mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    window_data * w = glfwGetWindowUserPointer(window);

    player_mouse_button_callback(w->player, button, action, mods);
}

void window_process_input(GLFWwindow *window, float delta_time)
{
    window_data * w = glfwGetWindowUserPointer(window);
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    camera_process_input(w->cam, window, delta_time);
}

GLFWwindow* window_init(int width, int height, camera * c, player * player){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL Window", NULL, NULL);
    if (window == NULL){
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  

    window_data * w_data = malloc(sizeof(*w_data));
    assert(w_data);
    w_data->cam = c;
    w_data->mouse_first = true;
    w_data->player = player;
    glfwSetWindowUserPointer(window, w_data);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, window_mouse_callback);
    glfwSetMouseButtonCallback(window, window_mouse_button_callback);

    return window;
}

bool window_should_close(GLFWwindow* w){
    return glfwWindowShouldClose(w);
}


void window_cleanup(GLFWwindow * window){
    free(glfwGetWindowUserPointer(window));
    glfwTerminate();
}