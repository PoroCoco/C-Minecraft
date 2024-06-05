#pragma once

#include <GLFW/glfw3.h>
#include <cglm/cglm.h> 

typedef struct camera{
    vec3 cameraPos;
    vec3 cameraWorldPos;
    vec3 cameraFront;
    vec3 cameraUp;

    float yaw;
    float pitch;
    float speed;

    mat4 view; // Maybe this will need a mutex to protect it during the glfw callback
} camera;

camera * camera_init();

void camera_process_input(camera * c, GLFWwindow * window, float delta_time);

void camera_mouse_callback(camera * c, double x_pos, double y_pos);

void camera_cleanup(camera * c);