#version 330 core

#define NR_POINT_LIGHTS 4

in vec3 vNormal;
in vec3 vVC;
in vec2 vTexCoord;

out vec4 fColor;

uniform vec3 viewPos;

struct Material{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
uniform Material material;

struct DirLight{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform DirLight dirLight;

struct PointLight{
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform PointLight pointLights[NR_POINT_LIGHTS];

struct SpotLight{
	vec3 position;
	vec3 direction;

	float constant;
	float linear;
	float quadratic;
	float cutOff;
	float outerCutOff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform SpotLight spotLight;

vec3 CalcDirLight(DirLight light, vec3 nNormal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 nNormal, vec3 vVC, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 nNormal, vec3 vVC, vec3 viewDir);

void main() {
	vec3 nNormal = normalize(vNormal);
	vec3 viewDir = normalize(viewPos - vVC);
	vec3 result = CalcDirLight(dirLight, nNormal, viewDir);
	for (int i = 0; i < NR_POINT_LIGHTS; i++){
		result += CalcPointLight(pointLights[i], nNormal, vVC, viewDir);
	}
	result += CalcSpotLight(spotLight, nNormal, vVC, viewDir);
	fColor = vec4(result, 1.f);
}

vec3 CalcDirLight(DirLight light, vec3 nNormal, vec3 viewDir){
	vec3 nLightDir = normalize(-light.direction);
	float diff = max(dot(nNormal, nLightDir), 0.0f);
	vec3 reflectDir = reflect(-nLightDir, nNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	vec3 ambient = vec3(texture(material.diffuse, vTexCoord)) * light.ambient;
	vec3 diffuse = (vec3(texture(material.diffuse, vTexCoord))* diff) * light.diffuse;
	vec3 specular = (vec3(texture(material.specular, vTexCoord)) * spec) * light.specular;
	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 nNormal, vec3 vVC, vec3 viewDir){
	vec3 nLightDir = normalize(light.position - vVC);
	float diff = max(dot(nNormal, nLightDir), 0.0f);
	vec3 reflectDir = reflect(-nLightDir, nNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	float lightDist = length(light.position - vVC);
	float attenuation = 1.f/(light.constant + light.linear * lightDist 
		+ light.quadratic * lightDist * lightDist);
	vec3 ambient = vec3(texture(material.diffuse, vTexCoord)) * light.ambient;
	vec3 diffuse = (vec3(texture(material.diffuse, vTexCoord))* diff) * light.diffuse;
	vec3 specular= (vec3(texture(material.specular, vTexCoord)) * spec) * light.specular;
	ambient*=attenuation;
	diffuse*=attenuation;
	specular*=attenuation;
	return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 nNormal, vec3 vVC, vec3 viewDir){
	vec3 nLightDir = normalize(light.position - vVC);
	float theta = dot(nLightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.f, 1.f);

	vec3 ambient = vec3(texture(material.diffuse, vTexCoord)) * light.ambient;

	float diff = max(dot(nNormal, nLightDir), 0.0f);
	vec3 diffuse = (vec3(texture(material.diffuse, vTexCoord))* diff) * light.diffuse;

	vec3 reflectDir = reflect(-nLightDir, nNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular= (vec3(texture(material.specular, vTexCoord)) * spec) * light.specular;

	float lightDist = length(light.position - vVC);
	float attenuation = 1.f/(light.constant + light.linear * lightDist 
		+ light.quadratic * lightDist * lightDist);
	diffuse*=intensity;
	specular*=intensity;
	ambient*=attenuation;
	diffuse*=attenuation;
	specular*=attenuation;
	return (ambient + diffuse + specular);
}
