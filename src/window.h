#pragma once

#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <camera.h>
#include <player.h>

typedef struct window_data {
    camera * cam;
    player * player;
    double mouse_last_x;
    double mouse_last_y;
    bool mouse_first;
} window_data;

// Add player to struct field
GLFWwindow* window_init(int width, int height, camera * c);

bool window_should_close();

void window_cleanup(GLFWwindow* window);

void window_process_input(GLFWwindow* window, float delta_time);
