#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

//=================IN==================
in vec2 texCoords;
in vec3 Normal;
in vec3 FragmentPos;
//=====================================
//================OUT==================
out vec4 color;
//=====================================
//==============UNIFORM================
//texture
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float transparency;

//material and light components
uniform Material material;
uniform Light light;

//others
uniform vec3 viewPos;
//=====================================

void main()
{
    //Ambient component of light
    vec3 ambient = material.ambient * light.ambient;

    //Diffuse component of light
    vec3 nNormal = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragmentPos);
    float diff = max(dot(nNormal,lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * material.diffuse;

    //Specular component of light
    vec3 viewDir = normalize(viewPos - FragmentPos);
    vec3 reflectDir = reflect(-lightDir, nNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);   //this is basically an intencity of the shining
    vec3 specular = light.specular * spec * material.specular;

    vec3 result = ambient + diffuse + specular;
    color = mix(texture(texture1, texCoords), texture(texture2, vec2(1.0-texCoords.x,texCoords.y)), transparency) * vec4(result, 1.0f);
}