#version 330 core

in vec3 ourColor;

out vec4 FragColor;

// uniform vec4 ourColor;

void main()
{
    FragColor = vec4(ourColor, 1.0f);
    // FragColor = vec4(1.0f, 0.5f, 0.2 f, 1.0f);
}