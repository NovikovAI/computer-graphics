#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
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
//material and light components
uniform Material material;
uniform Light light;
//others
uniform vec3 viewPos;
uniform float time;
//=====================================

void main()
{
    //Ambient component of light
    vec3 ambient = light.ambient * texture(material.diffuse, texCoords).rgb;

    //Diffuse component of light
    vec3 nNormal = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragmentPos);
    float diff = max(dot(nNormal,lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoords).rgb;

    //Specular component of light
    vec3 viewDir = normalize(viewPos - FragmentPos);
    vec3 reflectDir = reflect(-lightDir, nNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);   //this is basically an intencity of the shining
    vec3 specular = light.specular * spec * texture(material.specular, texCoords).rgb;

    //emission
    vec3 emission = vec3(0.0);
    if (texture(material.specular, texCoords).r == 0.0)
    {
        emission = texture(material.emission, texCoords + vec2(0.0,time / 5.0)).rgb;   /*moving */
        emission = emission * vec3(0.0, 0.0, 1.0);
        emission = emission * (sin(time) * 0.5 + 0.5) * 10.0;
    }

    vec3 result = ambient + diffuse + specular + emission;
    color = vec4(result, 1.0f);
}