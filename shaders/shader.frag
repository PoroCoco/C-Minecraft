#version 330 core

in float texLight;
in vec3 texCoord;

out vec4 FragColor;

uniform sampler2DArray texture1;
void main()
{
    // FragColor = mix(texture(texture1, texCoord), texture(texture2, texCoord), 0.2);
    FragColor = texture(texture1, texCoord) * texLight ;
    // FragColor = vec4(ourColor, 1.0f);
}