#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;
in vec3 fPosition;

out vec4 fColor;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;

//point lights
#define NR_POINT_LIGHTS 7
uniform vec3 pointLightPositions[NR_POINT_LIGHTS];
uniform vec3 pointLightColors[NR_POINT_LIGHTS];

uniform mat4 view;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//base color
uniform bool useTexture;
uniform bool hasDiffuseTexture;
uniform vec3 baseColor;


vec3 ambient;
float ambientStrength = 0.05f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

vec3 normal;
vec3 lightDire;
vec3 outDbg;

void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	normal = normalEye;
	outDbg = normal;
	//compute light direction
	vec3 lightDirN = normalize(lightDir);
	lightDire = lightDirN;
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}

vec3 computePointLights(vec3 lightPos, vec3 lightCol){
	vec3 cameraPosEye = vec3(0.0f);
	vec4 lightPosEye = view * vec4(lightPos, 1.0f);
	vec3 normalEye = normalize(fNormal);

	//direction from frag to light
	vec3 lightDir = normalize(lightPosEye.xyz - fPosEye.xyz);

	//compute diffuse light
	float diff = max(dot(normalEye, lightDir), 0.0f);

	//compute specular
	vec3 viewDir = normalize(cameraPosEye - fPosEye.xyz);
	vec3 reflection = reflect(-lightDir, normalEye);
	float spec = pow(max(dot(viewDir, reflection), 0.0f), shininess);

	//attenuation (decreses with distance)
	float distance = length(lightPosEye.xyz - fPosEye.xyz);
	float atten = 1.0f / (1.0f + 1.5f * distance + 3.0f * (distance * distance));

	vec3 diffLight = diff * lightCol * atten;
	vec3 specLight = spec * lightCol * atten * specularStrength;

	return diffLight + specLight;
}

float computeShadow(){
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
    normalizedCoords = normalizedCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
    float currentDepth = normalizedCoords.z;
    if (normalizedCoords.z > 1.0f)
        return 0.0f;

    float bias = max(0.005f * (1.0f - dot(normal, lightDire)), 0.0001f);

    float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
    return shadow;
}

float computeFog(){
	float fogDensity = 0.4f;
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

	return clamp(fogFactor, 0.0f, 1.0f);

}

void main() 
{
	computeLightComponents();
	
	vec3 diffColor;
	vec3 specColor;

	if(useTexture) {
		//with texture -> we sample the colors
		diffColor = texture(diffuseTexture, fTexCoords).rgb;
		specColor = texture(specularTexture, fTexCoords).rgb;
	}else{
		//without texture -> we use the uniform color
		diffColor = baseColor;
		specColor = vec3(1.0f);
	}

	//vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);//orange
	
	ambient *= diffColor;
	diffuse *= diffColor;
	specular *= specColor;

	float shadow = computeShadow();

	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);

	//add  point lights
	vec3 pointLight = vec3(0.0f);
	for (int i = 0; i < NR_POINT_LIGHTS; i++){
		pointLight += computePointLights(pointLightPositions[i], pointLightColors[i]);
	}

	color += pointLight * diffColor;

	//vec3 color = min((ambient + diffuse) + specular, 1.0f);
    float fogFactor = computeFog();
	vec4 fogColor = vec4(0.01f, 0.01f, 0.05f, 1.0f);
	fColor = fogColor * (1 - fogFactor) + vec4(color * fogFactor, 1.0f);
    //fColor = vec4(color, 1.0f);
    //fColor = vec4(outDbg, 1.0f);
}
