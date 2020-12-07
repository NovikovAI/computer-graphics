#version 330 core
in vec2 texCoords;

out vec4 color;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float transparency;

void main()
{
    color = mix(texture(texture1, texCoords), texture(texture2, vec2(1.0-texCoords.x,texCoords.y)), transparency);
}