#version 420 core

#define NUM_MATERIAL_COMPONENTS 5

const uint BASE_COMPONENT = 0;
const uint EMISSIVE_COMPONENT = 1;
const uint METALLIC_COMPONENT = 2;
const uint NORMAL_COMPONENT = 3;
const uint ROUGHNESS_COMPONENT = 4;

#define MAX_NUM_POINT_LIGHTS 8
#define MAX_NUM_SPOTLIGHTS 8
#define MAX_NUM_SHADOW_POINT_LIGHTS 2

in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragMaterialUV;
in vec2 fragMaskUV;

out vec4 color;

uniform vec3 cameraPosition;

uniform sampler2D material[NUM_MATERIAL_COMPONENTS];
uniform vec3 materialValues[NUM_MATERIAL_COMPONENTS];
uniform sampler2D materialMask;
uniform sampler2D opacityMask;
uniform sampler2D collectionMaterial[NUM_MATERIAL_COMPONENTS];
uniform vec3 collectionMaterialValues[NUM_MATERIAL_COMPONENTS];
uniform sampler2D grungeMaterial[NUM_MATERIAL_COMPONENTS];
uniform vec3 grungeMaterialValues[NUM_MATERIAL_COMPONENTS];
uniform sampler2D wearMaterial[NUM_MATERIAL_COMPONENTS];
uniform vec3 wearMaterialValues[NUM_MATERIAL_COMPONENTS];

uniform bool useCustomColor;
uniform vec3 customColor;

struct DirectionalLight
{
	vec3 color;
	vec3 ambient;
	vec3 direction;
};

struct PointLight
{
	vec3 color;
	vec3 ambient;
	vec3 position;
	float radius;
	int shadowIndex;
};

struct Spotlight
{
	vec3 color;
	vec3 ambient;
	vec3 position;
	vec3 direction;
	float radius;
	vec2 size;
};

uniform uint numDirectionalLights;
uniform DirectionalLight directionalLight;

uniform uint numPointLights;
uniform PointLight pointLights[MAX_NUM_POINT_LIGHTS];

uniform uint numSpotlights;
uniform Spotlight spotlights[MAX_NUM_SPOTLIGHTS];

uniform samplerCube pointLightShadowMaps[MAX_NUM_SHADOW_POINT_LIGHTS];
uniform float shadowPointLightFarPlanes[MAX_NUM_SHADOW_POINT_LIGHTS];

const vec3 sampleOffsetDirections[20] = vec3[](
   vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
   vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
   vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
   vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3( 0, 1, -1));

vec3 getDirectionalLightColor(
	DirectionalLight light,
	vec3 normal,
	vec3 diffuseTextureColor);
vec3 getPointLightColor(
	PointLight light,
	vec3 normal,
	vec3 position,
	vec3 diffuseTextureColor);
vec3 getSpotlightColor(
	Spotlight light,
	vec3 normal,
	vec3 position,
	vec3 diffuseTextureColor);

float getShadow(
	vec3 position,
	vec3 lightPosition,
	float farPlane,
	samplerCube shadowMap);

void main()
{
	if (useCustomColor)
	{
		color = vec4(customColor, 1);
		return;
	}

	vec3 diffuseTextureColor = vec3(texture(
		material[BASE_COMPONENT],
		fragMaterialUV));

	vec3 finalColor = vec3(0.0);

	if (numDirectionalLights == 1)
	{
		finalColor += getDirectionalLightColor(
			directionalLight,
			fragNormal,
			diffuseTextureColor);
	}

	for (uint i = 0; i < numPointLights; i++)
	{
		finalColor += getPointLightColor(
			pointLights[i],
			fragNormal,
			fragPosition,
			diffuseTextureColor);
	}

	for (uint i = 0; i < numSpotlights; i++)
	{
		finalColor += getSpotlightColor(
			spotlights[i],
			fragNormal,
			fragPosition,
			diffuseTextureColor);
	}

	color = vec4(finalColor, 1.0);
}

vec3 getDirectionalLightColor(
	DirectionalLight light,
	vec3 normal,
	vec3 diffuseTextureColor)
{
	vec3 lightDirection = normalize(-light.direction);

	float diffuseValue = max(dot(normal, lightDirection), 0.0);

	vec3 ambientColor = light.ambient * diffuseTextureColor;
	vec3 diffuseColor = light.color * diffuseValue * diffuseTextureColor;

	return ambientColor + diffuseColor;
}

vec3 getPointLightColor(
	PointLight light,
	vec3 normal,
	vec3 position,
	vec3 diffuseTextureColor)
{
	vec3 lightDirection = normalize(light.position - position);

	float diffuseValue = max(dot(normal, lightDirection), 0.0);
	float distance = length(light.position - position);

	float attenuation = clamp(
		1.0 - (distance * distance) / (light.radius * light.radius),
		0.0,
		1.0);
	attenuation *= attenuation;

	vec3 ambientColor = light.ambient * diffuseTextureColor * attenuation;
	vec3 diffuseColor =
		light.color * diffuseValue * diffuseTextureColor * attenuation;

	float shadow = 0.0;
	if (light.shadowIndex > -1)
	{
		shadow = getShadow(
			fragPosition,
			light.position,
			shadowPointLightFarPlanes[light.shadowIndex],
			pointLightShadowMaps[light.shadowIndex]);
	}

	shadow = 1.0 - shadow;
	return ambientColor + shadow * diffuseColor;
}

vec3 getSpotlightColor(
	Spotlight light,
	vec3 normal,
	vec3 position,
	vec3 diffuseTextureColor)
{
	vec3 lightDirection = normalize(light.position - position);
	float diffuseValue = max(dot(normal, lightDirection), 0.0);

	float distance = length(light.position - position);

	float attenuation = clamp(
		1.0 - (distance * distance) / (light.radius * light.radius),
		0.0,
		1.0);
	attenuation *= attenuation;

	float theta = dot(lightDirection, normalize(-light.direction));
	float epsilon = light.size.x - light.size.y;
	float intensity = clamp((theta - light.size.y) / epsilon, 0.0, 1.0);

	vec3 ambientColor = light.ambient * diffuseTextureColor *
		attenuation * intensity;
	vec3 diffuseColor = light.color * diffuseValue * diffuseTextureColor *
		attenuation * intensity;

	return ambientColor + diffuseColor;
}

float getShadow(
	vec3 position,
	vec3 lightPosition,
	float farPlane,
	samplerCube shadowMap)
{
	vec3 lightSpacePosition = position - lightPosition;
	float currentDepth = length(lightSpacePosition);

	float shadow = 0.0;
	float bias = 0.15;
	uint numSamples = 20;
	float cameraDistance = length(cameraPosition - position);
	float diskRadius = (1.0 + (cameraDistance / farPlane)) / 25.0;

	for (uint i = 0; i < numSamples; i++)
	{
		vec3 offset = sampleOffsetDirections[i] * diskRadius;
		float closestDepth = texture(
			shadowMap,
			lightSpacePosition + offset).r * farPlane;

		if (currentDepth - bias > closestDepth)
		{
			shadow += 1.0;
		}
	}

	shadow /= float(numSamples);
	return shadow;
}