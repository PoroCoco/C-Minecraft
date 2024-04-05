#version 330 core

layout (location = 0) in vec3 inPos;    
layout (location = 1) in vec3 inPosOffet;           //Instanced
layout (location = 2) in float inTexId;             //Instanced
layout (location = 3) in float inRotationIndex;     //Instanced
layout (location = 4) in vec2 inScale;              //Instanced

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 transfromMatrices[6];
uniform float chunkYOffset;

out vec3 texCoord;
out float texLight;


/*Square vertices
4----2,3
|     |
0,5---1
*/
void main()
{
    vec3 scaledPos = inPos;

    texCoord = vec3(0,0,0);

    // Translate vertex texture given the greedy meshing scale
    if (gl_VertexID == 1){
        texCoord = vec3(inScale.x , 0, inTexId);
    }else if (gl_VertexID == 2 || gl_VertexID == 3){
        texCoord = vec3(inScale.x , inScale.y, inTexId);
    }else if (gl_VertexID == 4){
        texCoord = vec3(0,  inScale.y , inTexId);
    }else if (gl_VertexID == 5 || gl_VertexID == 0){
        texCoord = vec3(0, 0, inTexId);
    }


    // Translate vertex pos given the greedy meshing scale
    if (int(inRotationIndex) == 2 || int(inRotationIndex) == 4 ){ // North or East
        if (gl_VertexID == 4){
            scaledPos = vec3(-inScale.x+1, inScale.y, inPos.z);
        }else if (gl_VertexID == 0 || gl_VertexID == 5){
            scaledPos = vec3(-inScale.x+1, inPos.y, inPos.z);
        }else if (gl_VertexID == 2 || gl_VertexID == 3){
            scaledPos = vec3(inPos.x, inScale.y, inPos.z);
        }
    }else if (int(inRotationIndex) == 3 || int(inRotationIndex) == 5 ){  // South or West
        if (gl_VertexID == 1){
            scaledPos = vec3(inScale.x, inPos.y, inPos.z);
        }else if (gl_VertexID == 2 || gl_VertexID == 3){
            scaledPos = vec3(inScale.x, inScale.y, inPos.z);
        }else if (gl_VertexID == 4){
            scaledPos = vec3(inPos.x, inScale.y, inPos.z);
        }
    }else if (int(inRotationIndex) == 0 ){  // Top
        if (gl_VertexID == 0 || gl_VertexID == 5){
            scaledPos = vec3(inPos.x, inPos.y-inScale.y+1, inPos.z);
        }else if (gl_VertexID == 1){
            scaledPos = vec3(inPos.x+inScale.x-1, inPos.y-inScale.y+1, inPos.z);
        }else if (gl_VertexID == 2 || gl_VertexID == 3){
            scaledPos = vec3(inPos.x+inScale.x-1, inPos.y, inPos.z);
        }else if (gl_VertexID == 4){
            scaledPos = vec3(inPos.x, inPos.y, inPos.z);
        }
    }else { // Bottom 
        if (gl_VertexID == 1){
            scaledPos = vec3(inScale.x, inPos.y, inPos.z);
        }else if (gl_VertexID == 2 || gl_VertexID == 3){
            scaledPos = vec3(inScale.x, inScale.y, inPos.z);
        }else if (gl_VertexID == 4){
            scaledPos = vec3(inPos.x, inScale.y, inPos.z);
        }
    }

    vec4 r = transfromMatrices[int(inRotationIndex)] * vec4(scaledPos, 1.0);


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
}