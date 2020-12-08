#version 330 core
in vec2 texCoords;
in vec3 Normal;
in vec3 FragmentPos;

out vec4 color;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float transparency;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    //Ambient component of light
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColor;

    //Diffuse component of light
    vec3 nNormal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragmentPos);
    float diff = max(dot(nNormal,lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    //Specular component of light
    float specularStrength = 0.5;                               //overall intencity
    vec3 viewDir = normalize(viewPos - FragmentPos);
    vec3 reflectDir = reflect(-lightDir, nNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);   //this is basically an intencity of the shining
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    color = mix(texture(texture1, texCoords), texture(texture2, vec2(1.0-texCoords.x,texCoords.y)), transparency) * vec4(result, 1.0f);
}