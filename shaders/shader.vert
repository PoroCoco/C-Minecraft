#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor; // the color variable has attribute position 1

uniform vec4 ourMove;

out vec3 ourColor;

void main()
{
    gl_Position = vec4(aPos.x+ourMove.x, aPos.y+ourMove.y, aPos.z, 1.0);
    ourColor = aColor;
}