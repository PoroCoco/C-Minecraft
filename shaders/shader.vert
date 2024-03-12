#version 330 core

layout (location = 0) in vec3 inPos;    
layout (location = 1) in vec2 inTexStep;
layout (location = 2) in vec3 inPosOffet;       //Instanced
layout (location = 3) in vec2 inTexStart;       //Instanced
layout (location = 4) in float inRotationIndex;     //Instanced

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 transfromMatrices[6];
uniform float chunkYOffset;

out vec2 texCoord;
out float texLight;

void main()
{
    vec4 r = transfromMatrices[int(inRotationIndex)] * vec4(inPos, 1.0);
    if (int(inRotationIndex) == 0){      // Top
        texLight = 1.0;
    }
    else if (int(inRotationIndex) == 1){ // Bottom
        texLight = 0.6;
    }
    else if (int(inRotationIndex) == 2){ // North
        texLight = 0.90;
    }
    else if (int(inRotationIndex) == 3){ // South
        texLight = 0.8;
    }
    else if (int(inRotationIndex) == 4){ // East
        texLight = 0.87;
    }
    else if (int(inRotationIndex) == 5){ // West
        texLight = 0.7;
    }
    gl_Position = projection * view *  model * vec4(r.x + inPosOffet.x, r.y + inPosOffet.y + chunkYOffset, r.z + inPosOffet.z, 1.0);
    texCoord = inTexStart + inTexStep;
}