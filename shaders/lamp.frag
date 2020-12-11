#version 330 core
out vec4 color;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;

void main()
{
    vec3 result = ambient + diffuse + specular * 0.5;
    color = vec4(result, 1.0f);
}