#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include <glad/glad.h> 

#include <camera.h>

void camera_mouse_callback(camera * c, double x_offset, double y_offset){
    c->yaw += (float)x_offset;
    c->pitch += (float)y_offset;

    if(c->pitch > 89.0f){
        c->pitch = 89.0f;
    }
    if(c->pitch < -89.0f){
        c->pitch = -89.0f;
    }

    c->cameraFront[0] = cosf(glm_rad(c->yaw)) * cosf(glm_rad(c->pitch));
    c->cameraFront[1] = sinf(glm_rad(c->pitch));
    c->cameraFront[2] = sinf(glm_rad(c->yaw)) * cosf(glm_rad(c->pitch));
    glm_vec3_norm(c->cameraFront);

    vec3 tmp;
    glm_vec3_add(c->cameraPos, c->cameraFront, tmp);
    glm_lookat(c->cameraPos, tmp, c->cameraUp, c->view);
}

camera * camera_init(){
    camera * c = malloc(sizeof(*c));
    assert(c);
    c->yaw = -90.0f;
    c->pitch = 0.0f;
    
    c->cameraFront[0] = cosf(glm_rad(c->yaw)) * cosf(glm_rad(c->pitch));
    c->cameraFront[1] = sinf(glm_rad(c->pitch));
    c->cameraFront[2] = sinf(glm_rad(c->yaw)) * cosf(glm_rad(c->pitch));

    vec3 cameraPos = {0.0f, 0.0f,  3.0f};
    vec3 cameraUp = {0.0f, 1.0f,  0.0f};
    glm_vec3_copy(cameraPos, c->cameraPos);
    glm_vec3_copy(cameraUp, c->cameraUp);

    vec3 center;
    glm_vec3_add(c->cameraPos, c->cameraFront, center);
    glm_lookat(c->cameraPos, center, c->cameraUp, c->view);

    return c;
}

void camera_process_input(camera * c, GLFWwindow * window, float delta_time){
    const float cameraSpeed = CAM_SPEED * delta_time; 
    vec3 tmp;
    bool moved = false;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        glm_vec3_muladds(c->cameraFront, cameraSpeed, c->cameraPos);
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        glm_vec3_mulsubs(c->cameraFront, cameraSpeed, c->cameraPos);
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        glm_vec3_cross(c->cameraFront, c->cameraUp, tmp);
        glm_vec3_norm(tmp);
        glm_vec3_mulsubs(tmp, cameraSpeed, c->cameraPos);
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        glm_vec3_cross(c->cameraFront, c->cameraUp, tmp);
        glm_vec3_norm(tmp);
        glm_vec3_muladds(tmp, cameraSpeed, c->cameraPos);
        moved = true;
    }

    // Updates the view matrix
    if (moved){
        glm_vec3_add(c->cameraPos, c->cameraFront, tmp);
        glm_lookat(c->cameraPos, tmp, c->cameraUp, c->view);
    }
}


void camera_cleanup(camera * c){
    free(c);
}