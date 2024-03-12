#include <glad/glad.h> 
#include <shader.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>

#define DEBUG_GL(command) do { \
    command; \
    GLenum error = glGetError(); \
    if (error != GL_NO_ERROR) { \
        fprintf(stderr, "OpenGL Error at %s:%d - Code %d\n", __FILE__, __LINE__, error); \
    } \
} while(0)

static char infoLog[512];

char * read_file(const char * path){
    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *s = malloc(fsize + 1);
    assert(s);
    fread(s, fsize, 1, f);
    fclose(f);

    s[fsize] = '\0';

    return s;
}

void check_shader_compilation(unsigned int shader_id){
    int success;
    DEBUG_GL(glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success));
    if(!success){
        DEBUG_GL(glGetShaderInfoLog(shader_id, 512, NULL, infoLog));
        fprintf(stderr,"ERROR::SHADER::COMPILATION_FAILED %s\n", infoLog);
    }
}

void check_program_compilation(unsigned int program_id){
    int success;
    DEBUG_GL(glGetProgramiv(program_id, GL_LINK_STATUS, &success));
    if(!success){
        DEBUG_GL(glGetProgramInfoLog(program_id, 512, NULL, infoLog));
        fprintf(stderr,"ERROR::SHADER::LINKING_FAILED %s\n", infoLog);
    }
}

shader * shader_init(const char * vertexPath, const char * fragmentPath){
    shader * s = malloc(sizeof(*s));
    assert(s);

    // Loading the shaders    
    char const * vertexShaderSource = read_file(vertexPath);
    char const * fragmentShaderSource = read_file(fragmentPath);

    // Shaders creations
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    DEBUG_GL(glShaderSource(vertexShader, 1, &vertexShaderSource, NULL));
    DEBUG_GL(glCompileShader(vertexShader));
    check_shader_compilation(vertexShader);
    
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    DEBUG_GL(glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL));
    DEBUG_GL(glCompileShader(fragmentShader));
    check_shader_compilation(fragmentShader);
    
    // Creating the shader program with the 2 shaders
    s->id = glCreateProgram();
    DEBUG_GL(glAttachShader(s->id, vertexShader));
    DEBUG_GL(glAttachShader(s->id, fragmentShader));
    DEBUG_GL(glLinkProgram(s->id));
    check_program_compilation(s->id);

    // Deleting the shaders now that they are linked
    DEBUG_GL(glDeleteShader(vertexShader));
    DEBUG_GL(glDeleteShader(fragmentShader));  

    return s;
}

void shader_use(shader const *s){
    DEBUG_GL(glUseProgram(s->id));
}
void shader_set_float4(shader *s, const char * var_name, float value1, float value2, float value3, float value4){
    int uniformLocation = glGetUniformLocation(s->id, var_name);
    if (uniformLocation == -1){
        fprintf(stderr,"Invalid uniform name : %s\n", var_name);
    }
    DEBUG_GL(glUniform4f(uniformLocation, value1, value2, value3, value4));
}

void shader_set_int(shader *s, const char * var_name, int value){
    int uniformLocation = glGetUniformLocation(s->id, var_name);
    if (uniformLocation == -1){
        fprintf(stderr,"Invalid uniform name : %s\n", var_name);
    }
    DEBUG_GL(glUniform1i(uniformLocation, value));
}

void shader_set_float(shader *s, const char * var_name, float value){
    int uniformLocation = glGetUniformLocation(s->id, var_name);
    if (uniformLocation == -1){
        fprintf(stderr,"Invalid uniform name : %s\n", var_name);
    }
    DEBUG_GL(glUniform1f(uniformLocation, value));
}

void shader_set_m4(shader *s, const char * var_name, mat4 value){
    int uniformLocation = glGetUniformLocation(s->id, var_name);
    if (uniformLocation == -1){
        fprintf(stderr,"Invalid uniform name : %s\n", var_name);
    }
    DEBUG_GL(glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, (float*)value));
}

void shader_set_transform_matrices(shader *s, const char * var_name, mat4 *rotations){
    int uniformLocation = glGetUniformLocation(s->id, var_name);
    if (uniformLocation == -1){
        fprintf(stderr,"Invalid uniform name : %s\n", var_name);
    }
    DEBUG_GL(glUniformMatrix4fv(uniformLocation, 6, GL_FALSE, (float*)rotations[0]));
}


void shader_cleanup(shader * s){
    DEBUG_GL(glDeleteProgram(s->id));
}
