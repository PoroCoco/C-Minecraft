#version 330 core

layout (location = 0) in vec3 inPos;    
layout (location = 1) in ivec2 inInstancePackedData;    //Instanced

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 transfromMatrices[6];
uniform float chunkYOffset;

out vec3 texCoord;
out float texLight;


/*Square vertices (GL_TRIANGLE):
4----2,3
|     |
0,5---1

Square vertices (GL_TRIANGLE_STRIP):
3----1
|    |
2----0
*/
void main()
{
    vec3 inPosOffet = vec3( (inInstancePackedData.x >> 0) & 0x3F,
                            (inInstancePackedData.x >> 6) & 0x7F,
                            (inInstancePackedData.x >> 13) & 0x3F);
    float inTexId = ((inInstancePackedData.x >> 19) & 0xFF);
    float inRotationIndex = ((inInstancePackedData.x >> 27) & 0x7);
    vec2 inScale = vec2(((inInstancePackedData.y >> 0) & 0x7F) +1,
                        ((inInstancePackedData.y >> 7) & 0x7F) +1);


    // Translate vertex texture given the greedy meshing scale
    if (gl_VertexID == 0){
        texCoord = vec3(inScale.x , 0, inTexId);
    }else if (gl_VertexID == 1){
        texCoord = vec3(inScale.x , inScale.y, inTexId);
    }else if (gl_VertexID == 3){
        texCoord = vec3(0,  inScale.y , inTexId);
    }else if (gl_VertexID == 2){
        texCoord = vec3(0, 0, inTexId);
    }else{
        texCoord = vec3(0,0,0);
    }


    // Translate vertex pos given the greedy meshing scale
    vec3 scaledPos = inPos;
    if (int(inRotationIndex) == 2 || int(inRotationIndex) == 4 ){ // North or East
        if (gl_VertexID == 3){
            scaledPos = vec3(-inScale.x+1, inScale.y, inPos.z);
        }else if (gl_VertexID == 2){
            scaledPos = vec3(-inScale.x+1, inPos.y, inPos.z);
        }else if (gl_VertexID == 1){
            scaledPos = vec3(inPos.x, inScale.y, inPos.z);
        }
    }else if (int(inRotationIndex) == 3 || int(inRotationIndex) == 5 ){  // South or West
        if (gl_VertexID == 0){
            scaledPos = vec3(inScale.x, inPos.y, inPos.z);
        }else if (gl_VertexID == 1){
            scaledPos = vec3(inScale.x, inScale.y, inPos.z);
        }else if (gl_VertexID == 3){
            scaledPos = vec3(inPos.x, inScale.y, inPos.z);
        }
    }else if (int(inRotationIndex) == 0 ){  // Top
        if (gl_VertexID == 2){
            scaledPos = vec3(inPos.x, inPos.y-inScale.y+1, inPos.z);
        }else if (gl_VertexID == 0){
            scaledPos = vec3(inPos.x+inScale.x-1, inPos.y-inScale.y+1, inPos.z);
        }else if (gl_VertexID == 1){
            scaledPos = vec3(inPos.x+inScale.x-1, inPos.y, inPos.z);
        }else if (gl_VertexID == 3){
            scaledPos = vec3(inPos.x, inPos.y, inPos.z);
        }
    }else { // Bottom 
        if (gl_VertexID == 0){
            scaledPos = vec3(inScale.x, inPos.y, inPos.z);
        }else if (gl_VertexID == 1){
            scaledPos = vec3(inScale.x, inScale.y, inPos.z);
        }else if (gl_VertexID == 3){
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