#version 330 core

//==============STRUCTS================
struct Material {
	sampler2D diffuse;
	sampler2D specular;
	sampler2D emission;
	float shininess;
}; 

struct DirectLight {
	vec3 direction;
  
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {    
	vec3 position;
	
	float constant;
	float linear;
	float quadratic;  

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Spotlight {
	bool enabled;
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;
  
	float constant;
	float linear;
	float quadratic;
  
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;       
};
//=====================================
//=================IN==================
in vec2 texCoords;
in vec3 Normal;
in vec3 FragmentPos;
in vec4 FragPosLightSpace;
//=====================================
//================OUT==================
out vec4 color;
//=====================================
//==============UNIFORM================
#define MAX_OF_POINT_LIGHTS 4

//material and light components
uniform Material material;
uniform DirectLight directLight;
uniform PointLight pointLights[MAX_OF_POINT_LIGHTS];
uniform Spotlight spotlight;

//others
uniform sampler2D shadowMap;
uniform vec3 viewPos;
uniform float time;
uniform bool lampsLightEnabled;
uniform int numberOfPointLights;
//=====================================
//====================================FUNCTIONS===============================================
vec3 CalculateDirectLight(DirectLight light, vec3 normal, vec3 viewDir, float shadow)
{
	vec3 resLight = vec3(0.0, 0.0, 0.0);

	vec3 lightDir = normalize(-light.direction);
	//diffuse component
	float diff = max(dot(normal, lightDir), 0.0);
	//specular component
	/* Phong model
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	*/
	//Blinn-Phong model
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir),0.0), 0.25 * material.shininess);

	vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoords));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoords));
	
	resLight += ambient + (1.0 - shadow)*(diffuse + specular);
	
	return resLight;
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 resLight = vec3(0.0, 0.0, 0.0);
	
	vec3 lightDir = normalize(light.position - fragPos);
	//diffuse component
	float diff = max(dot(normal, lightDir), 0.0);
	//specular component
	/*Phong model
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	*/
	//Blinn-Phong model
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir),0.0), 2 * material.shininess);
	//attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

	vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoords));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoords));

	//applying attenuation
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	resLight += ambient + diffuse + specular;

	return resLight;
}

vec3 CalculateSpotlight(Spotlight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 resLight = vec3(0.0, 0.0, 0.0);

	vec3 lightDir = normalize(light.position - fragPos);
	//diffuse component
	float diff = max(dot(normal, lightDir), 0.0);
	//specular component
	/*Phong model
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	*/
	//Blinn-Phong model
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir),0.0), 2 * material.shininess);
	//intensity(for soft edges)
	float theta = dot(lightDir, normalize(-light.direction)); 
	float epsilon = (light.cutOff - light.outerCutOff);
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	//attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	vec3 ambient  = light.ambient * vec3(texture(material.diffuse, texCoords));
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoords).rgb;
	vec3 specular = light.specular * spec * texture(material.specular, texCoords).rgb;

	//applying intensity and attenuation
	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;

	resLight += ambient + diffuse + specular;
	
	return resLight;
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightPos)
{
    float shadow = 0.0;
	
	vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragmentPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // PCF
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}
//============================================================================================

void main()
{
	vec3 nNormal = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragmentPos);

	float shadow = ShadowCalculation(FragPosLightSpace, directLight.direction);                     

	//applying all light components
	vec3 result = CalculateDirectLight(directLight, nNormal, viewDir, shadow);

	if (lampsLightEnabled)
	{
		for (int i =0; i < MAX_OF_POINT_LIGHTS; i++)
		{
			result += CalculatePointLight(pointLights[i], nNormal, FragmentPos, viewDir);
			if (i > numberOfPointLights)
				break;
		}
	}

	if (spotlight.enabled)
		result += CalculateSpotlight(spotlight, nNormal, FragmentPos, viewDir);

	//emission
	vec3 emission = vec3(0.0);
	if (texture(material.specular, texCoords).r == 0.0)
	{
		emission = texture(material.emission, texCoords + vec2(0.0,time / 5.0)).rgb;   /*moving */
		emission = emission * vec3(0.0, 0.0, 1.0);
		emission = emission * (sin(time) * 0.5 + 0.5) * 10.0;
	}

	result += emission;  

	color = vec4(result, 1.0f);
}