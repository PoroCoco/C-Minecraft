#version 330 core

layout (location = 0) in vec3 inPos;    
layout (location = 1) in vec2 inTexStep;
layout (location = 2) in vec3 inPosOffet;       //Instanced
layout (location = 3) in vec2 inTexStart;       //Instanced
layout (location = 4) in float inRotationIndex;     //Instanced

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 rotationMatrices[6];
uniform float chunkYOffset;

out vec2 texCoord;

void main()
{
    vec4 r = rotationMatrices[int(inRotationIndex)]* vec4(inPos.x,inPos.y,inPos.z, 1.0);
    if (int(inRotationIndex) == 0){
        r = r + vec4(0.,0.,+1.,0.);
    }
    else if (int(inRotationIndex) == 1){
        r = r + vec4(0.,1.,0.,0.);
    }
    else if (int(inRotationIndex) == 2){
        r = r + vec4(1.,0.,1.,0.);
    }
    else if (int(inRotationIndex) == 3){

    }
    else if (int(inRotationIndex) == 4){
        r = r + vec4(0.,0.,1.,0.);
    }
    else if (int(inRotationIndex) == 5){
        r = r + vec4(1.,0.,0.,0.);
    }
    gl_Position = projection * view *  model * vec4(r.x + inPosOffet.x, r.y + inPosOffet.y + chunkYOffset, r.z + inPosOffet.z, 1.0);
    texCoord = inTexStart + inTexStep;
}