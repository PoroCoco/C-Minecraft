#include <glad/glad.h> 
#include <shader.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>

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
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(shader_id, 512, NULL, infoLog);
        fprintf(stderr,"ERROR::SHADER::COMPILATION_FAILED %s\n", infoLog);
    }
}

void check_program_compilation(unsigned int program_id){
    int success;
    glGetProgramiv(program_id, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(program_id, 512, NULL, infoLog);
        fprintf(stderr,"ERROR::SHADER::LINKING_FAILED %s\n", infoLog);
    }
}

shader * shader_init(const char * vertexPath, const char * fragmentPath){
    shader * s = malloc(sizeof(*s));
    assert(s);

    // Loading the shaders    
    char * vertexShaderSource = read_file(vertexPath);
    char * fragmentShaderSource = read_file(fragmentPath);

    // Shaders creations
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    check_shader_compilation(vertexShader);
    
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    check_shader_compilation(fragmentShader);
    
    // Creating the shader program with the 2 shaders
    s->id = glCreateProgram();
    glAttachShader(s->id, vertexShader);
    glAttachShader(s->id, fragmentShader);
    glLinkProgram(s->id);
    check_program_compilation(s->id);

    // Deleting the shaders now that they are linked
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);  

    return s;
}

void shader_use(shader const *s){
    glUseProgram(s->id);
}
void shader_set_float4(shader *s, const char * var_name, float value1, float value2, float value3, float value4){
    int vertexColorLocation = glGetUniformLocation(s->id, var_name);
    if (vertexColorLocation == -1){
        fprintf(stderr,"Invalid uniform name : %s\n", var_name);
    }
    glUniform4f(vertexColorLocation, value1, value2, value3, value4);
}

void shader_cleanup(shader * s){
    glDeleteProgram(s->id);
}
