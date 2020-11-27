#version 330 core
in vec3 myColor;
in vec2 texCoords;

out vec4 color;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    color = mix(texture(texture1, texCoords), texture(texture2, texCoords), 0.2) * vec4(myColor, 1.0f);
}