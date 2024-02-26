#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <assert.h>
#include <gpu.h>

static const float vertices_face_south[] = {
    0.f, 0.f,  1.f,  0.0f, 0.0f,
    1.f, 0.f,  1.f,  ATLAS_STEP, 0.0f,
    1.f,  1.f,  1.f,  ATLAS_STEP, ATLAS_STEP,
    1.f,  1.f,  1.f,  ATLAS_STEP, ATLAS_STEP,
    0.f,  1.f,  1.f,  0.0f, ATLAS_STEP,
    0.f, 0.f,  1.f,  0.0f, 0.0f,
};

gpu * gpu_init(atlas * atlas){
    gpu *gpu = malloc(sizeof(*gpu));
    gpu->atlas = atlas;

    // GLAD init
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return NULL;
    }    

    printf("Starting opengl object creation \n");
    // OpenGL Objects
    // Create the VAOs
    DEBUG_GL(glGenVertexArrays(TOTAL_CHUNKS, gpu->VAO));

    printf("vaoed \n");
    // Create the VBO 
    DEBUG_GL(glGenBuffers(1, &gpu->VBO_face));

    for (size_t i = 0; i < TOTAL_CHUNKS; i++){
        DEBUG_GL(glBindVertexArray(gpu->VAO[i]));
        glBindBuffer(GL_ARRAY_BUFFER, gpu->VBO_face);
        // Upload the data for a face
        DEBUG_GL(glBufferData(GL_ARRAY_BUFFER, sizeof(float)*ATTRIBUTE_PER_VERTEX*6, vertices_face_south, GL_STATIC_DRAW)); 
        // setting its attributes
        DEBUG_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, ATTRIBUTE_PER_VERTEX * sizeof(float), (void*)0));
        DEBUG_GL(glEnableVertexAttribArray(0));
        DEBUG_GL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, ATTRIBUTE_PER_VERTEX * sizeof(float), (void*)(3 * sizeof(float))));
        DEBUG_GL(glEnableVertexAttribArray(1));
    }

    // Create the Instances Buffer Attributes
    const unsigned int attribute_per_instance = 6; // x,y,z; texturex,texturey; position_index;
    DEBUG_GL(glGenBuffers(TOTAL_CHUNKS, gpu->IBA));
    for (size_t i = 0; i < TOTAL_CHUNKS; i++)
    {
        DEBUG_GL(glBindVertexArray(gpu->VAO[i]));
        glBindBuffer(GL_ARRAY_BUFFER, gpu->IBA[i]);
        // Pre-Allocate the buffer
        DEBUG_GL(glBufferData(GL_ARRAY_BUFFER, MAX_FACE_IN_CHUNK * sizeof(float) * attribute_per_instance, NULL, GL_STATIC_DRAW)); 
        DEBUG_GL(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
        DEBUG_GL(glEnableVertexAttribArray(2));
        DEBUG_GL(glVertexAttribDivisor(2, 1));
        DEBUG_GL(glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(3 * MAX_FACE_IN_CHUNK * sizeof(float))));
        DEBUG_GL(glEnableVertexAttribArray(3));
        DEBUG_GL(glVertexAttribDivisor(3, 1));
        DEBUG_GL(glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(int), (void*)(5 * MAX_FACE_IN_CHUNK * sizeof(float))));
        DEBUG_GL(glEnableVertexAttribArray(4));
        DEBUG_GL(glVertexAttribDivisor(4, 1));
    }


    printf("OpenGL Objects creation successful\n");
    return gpu;
}

void gpu_set_VAO(gpu* gpu, uint64_t vao_index){
    DEBUG_GL(glBindVertexArray(gpu->VAO[vao_index]));
}

void gpu_unload(gpu* gpu, uint64_t chunk_index){
    // Do nothing, you just need to upload on top of this now old data
}

// ToDo: Opti -> use subdata or even map it instead +  Check between static and dynamic perf for the buffer data
void gpu_upload(gpu* gpu, uint64_t chunk_index, chunk *c){
    DEBUG_GL(glBindVertexArray(gpu->VAO[chunk_index]));
    DEBUG_GL(glBindBuffer(GL_ARRAY_BUFFER, gpu->IBA[chunk_index]));
    float * instaces_offsets = chunk_get_faces_offsets(c, &(gpu->instances_count[chunk_index]));
    assert(MAX_FACE_IN_CHUNK > gpu->instances_count[chunk_index]); // Need to increase the max face per chunk
    DEBUG_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*gpu->instances_count[chunk_index], instaces_offsets));

    float * textures_start = chunk_get_textures(c, &(gpu->instances_count[chunk_index]), gpu->atlas);
    assert(MAX_FACE_IN_CHUNK > gpu->instances_count[chunk_index]); // Need to increase the max face per chunk
    DEBUG_GL(glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*MAX_FACE_IN_CHUNK*3, sizeof(float)*2*gpu->instances_count[chunk_index], textures_start));

    float * rotations_values = chunk_get_rotations_values(c, &(gpu->instances_count[chunk_index]));
    assert(MAX_FACE_IN_CHUNK > gpu->instances_count[chunk_index]); // Need to increase the max face per chunk
    DEBUG_GL(glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*MAX_FACE_IN_CHUNK*5, sizeof(float)*1*gpu->instances_count[chunk_index], rotations_values));
}

void gpu_draw(gpu* gpu, uint64_t index){
    gpu_set_VAO(gpu, index);
    DEBUG_GL(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, gpu->instances_count[index]));
}

void gpu_cleanup(gpu* gpu){
    
    DEBUG_GL(glDeleteVertexArrays(TOTAL_CHUNKS, gpu->VAO));
    DEBUG_GL(glDeleteBuffers(1, &gpu->VBO_face));
    DEBUG_GL(glDeleteBuffers(TOTAL_CHUNKS, gpu->IBA));

    free(gpu);
}