#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos*1000;
    gl_Position = projection * view * vec4(aPos*1000, 1.0);
}  