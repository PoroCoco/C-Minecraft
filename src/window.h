#pragma once

#include <GLFW/glfw3.h>
#include <stdbool.h>

GLFWwindow* window_init();

bool window_should_close();

void window_cleanup();

void window_process_input(GLFWwindow* window);
