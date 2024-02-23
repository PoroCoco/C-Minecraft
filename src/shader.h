#pragma once

#include <GLFW/glfw3.h>
#include <cglm/cglm.h> 
#include <stdbool.h>

typedef struct shader{
    unsigned int id;

} shader;


shader * shader_init(const char * vertexPath, const char * fragmentPath);

void shader_use(shader const *s);
void shader_set_float4(shader *s, const char * var_name, float value1, float value2, float value3, float value4);
// void shader_set_bool(shader *s, const char * var_name, bool value);
void shader_set_int(shader *s, const char * var_name, int value);
void shader_set_float(shader *s, const char * var_name, float value);
void shader_set_m4(shader *s, const char * var_name, mat4 value);
void shader_set_rotation_matrices(shader *s, const char * var_name, mat4 *rotations);

void shader_cleanup(shader * s);
