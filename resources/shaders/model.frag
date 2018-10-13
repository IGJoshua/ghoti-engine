#version 420 core
#extension GL_ARB_bindless_texture: require
#extension GL_ARB_gpu_shader_int64: require

#define NUM_MATERIAL_COMPONENTS 7

const uint AMBIENT_OCCLUSION_COMPONENT = 0;
const uint BASE_COMPONENT = 1;
const uint EMISSIVE_COMPONENT = 2;
const uint HEIGHT_COMPONENT = 3;
const uint METALLIC_COMPONENT = 4;
const uint NORMAL_COMPONENT = 5;
const uint ROUGHNESS_COMPONENT = 6;

#define MAX_NUM_POINT_LIGHTS 8
#define MAX_NUM_SPOTLIGHTS 8

#define MAX_NUM_SHADOW_POINT_LIGHTS 4
#define MAX_NUM_SHADOW_SPOTLIGHTS 4

struct DirectionalLight
{
	vec3 radiantFlux;
	vec3 direction;
};

struct PointLight
{
	vec3 radiantFlux;
	vec3 position;
	float radius;
	int shadowIndex;
};

struct Spotlight
{
	vec3 radiantFlux;
	vec3 position;
	vec3 direction;
	float radius;
	vec2 size;
	int shadowIndex;
};

in vec4 fragColor;
in vec3 fragPosition;
in vec4 fragDirectionalLightSpacePosition;
in vec4 fragSpotlightSpacePositions[MAX_NUM_SHADOW_SPOTLIGHTS];
in vec3 fragNormal;
in vec3 fragTangent;
in vec2 fragMaterialUV;
in vec2 fragMaskUV;

out vec4 color;

uniform vec3 cameraPosition;

uniform bool materialActive[NUM_MATERIAL_COMPONENTS];
uniform uint64_t material[NUM_MATERIAL_COMPONENTS];
uniform vec3 materialValues[NUM_MATERIAL_COMPONENTS];

uniform bool useCustomColor;
uniform vec3 customColor;

uniform uint numDirectionalLights;
uniform DirectionalLight directionalLight;

uniform uint numPointLights;
uniform PointLight pointLights[MAX_NUM_POINT_LIGHTS];

uniform uint numSpotlights;
uniform Spotlight spotlights[MAX_NUM_SPOTLIGHTS];

uniform uint numDirectionalLightShadowMaps;
uniform sampler2D directionalLightShadowMap;
uniform vec2 shadowDirectionalLightBiasRange;

uniform samplerCube pointLightShadowMaps[MAX_NUM_SHADOW_POINT_LIGHTS];
uniform float shadowPointLightBias;
uniform float shadowPointLightDiskRadius;

uniform sampler2D spotlightShadowMaps[MAX_NUM_SHADOW_SPOTLIGHTS];
uniform vec2 shadowSpotlightBiasRange;

const vec3 pcfSampleOffsets[20] = vec3[](
	vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
	vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
	vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
	vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
	vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3( 0, 1, -1));

vec2 getMaterialUV(vec3 viewDirection);

mat3 createTBNMatrix(vec3 normal, vec3 tangent);

vec3 getAlbedoTextureColor(vec2 uv);
vec3 getNormalTextureColor(vec2 uv);

vec3 getDirectionalLightColor(
	DirectionalLight light,
	vec3 normal,
	vec3 albedoTextureColor);
vec3 getPointLightColor(
	PointLight light,
	vec3 normal,
	vec3 position,
	vec3 albedoTextureColor);
vec3 getSpotlightColor(
	Spotlight light,
	vec3 normal,
	vec3 position,
	vec3 albedoTextureColor);

float getDirectionalShadow(
	vec4 lightSpacePosition,
	vec3 normal,
	vec3 lightDirection,
	float minBias,
	float maxBias,
	sampler2D shadowMap);
float getPointShadow(
	vec3 position,
	vec3 lightPosition,
	float farPlane,
	samplerCube shadowMap);

void main()
{
	if (useCustomColor)
	{
		color = vec4(customColor, 1.0);
		return;
	}

	vec3 viewDirection = normalize(cameraPosition - fragPosition);

	vec2 materialUV = getMaterialUV(viewDirection);

	vec3 albedoTextureColor = getAlbedoTextureColor(materialUV);
	vec3 normalTextureColor = getNormalTextureColor(materialUV);

	vec3 finalColor = vec3(0.0);

	if (numDirectionalLights == 1)
	{
		finalColor += getDirectionalLightColor(
			directionalLight,
			normalTextureColor,
			albedoTextureColor);
	}

	for (uint i = 0; i < numPointLights; i++)
	{
		finalColor += getPointLightColor(
			pointLights[i],
			normalTextureColor,
			fragPosition,
			albedoTextureColor);
	}

	for (uint i = 0; i < numSpotlights; i++)
	{
		finalColor += getSpotlightColor(
			spotlights[i],
			normalTextureColor,
			fragPosition,
			albedoTextureColor);
	}

	color = vec4(finalColor, 1.0);
}

vec2 getMaterialUV(vec3 viewDirection)
{
	if (!materialActive[HEIGHT_COMPONENT])
	{
		return fragMaterialUV;
	}

	return fragMaterialUV;
}

mat3 createTBNMatrix(vec3 normal, vec3 tangent)
{
	tangent = normalize(tangent - dot(tangent, normal) * normal);
	vec3 bitangent = cross(tangent, normal);
	return mat3(tangent, bitangent, normal);
}

vec3 getAlbedoTextureColor(vec2 uv)
{
	vec3 albedoTextureColor = fragColor.rgb;
	if (materialActive[BASE_COMPONENT])
	{
		sampler2D sampler = sampler2D(material[BASE_COMPONENT]);
		albedoTextureColor = vec3(texture(sampler, uv));
		albedoTextureColor *= materialValues[BASE_COMPONENT];
	}

	return albedoTextureColor;
}

vec3 getNormalTextureColor(vec2 uv)
{
	vec3 normalTextureColor = fragNormal;
	if (materialActive[NORMAL_COMPONENT])
	{
		sampler2D sampler = sampler2D(material[NORMAL_COMPONENT]);
		normalTextureColor = texture(sampler, uv).rgb;
		normalTextureColor = normalize(normalTextureColor * 2.0 - 1.0);
		normalTextureColor = normalize(
			normalTextureColor * materialValues[NORMAL_COMPONENT]);

		mat3 TBN = createTBNMatrix(fragNormal, fragTangent);
		normalTextureColor = normalize(TBN * normalTextureColor);
	}

	return normalTextureColor;
}

vec3 getDirectionalLightColor(
	DirectionalLight light,
	vec3 normal,
	vec3 albedoTextureColor)
{
	vec3 lightDirection = normalize(-light.direction);

	float diffuseValue = max(dot(normal, lightDirection), 0.0);

	vec3 ambientColor = 0.1 * albedoTextureColor;
	vec3 diffuseColor = light.radiantFlux * diffuseValue * albedoTextureColor;

	float shadow = 0.0;
	if (numDirectionalLightShadowMaps > 0)
	{
		shadow = getDirectionalShadow(
			fragDirectionalLightSpacePosition,
			normal,
			light.direction,
			shadowDirectionalLightBiasRange.x,
			shadowDirectionalLightBiasRange.y,
			directionalLightShadowMap);
	}

	shadow = 1.0 - shadow;
	return ambientColor + shadow * diffuseColor;
}

vec3 getPointLightColor(
	PointLight light,
	vec3 normal,
	vec3 position,
	vec3 albedoTextureColor)
{
	vec3 lightDirection = normalize(light.position - position);

	float diffuseValue = max(dot(normal, lightDirection), 0.0);
	float distance = length(light.position - position);

	float attenuation = clamp(
		1.0 - (distance * distance) / (light.radius * light.radius),
		0.0,
		1.0);
	attenuation *= attenuation;

	vec3 ambientColor = 0.1 * albedoTextureColor * attenuation;
	vec3 diffuseColor =
		light.radiantFlux * diffuseValue * albedoTextureColor * attenuation;

	float shadow = 0.0;
	if (light.shadowIndex > -1)
	{
		shadow = getPointShadow(
			fragPosition,
			light.position,
			light.radius,
			pointLightShadowMaps[light.shadowIndex]);
	}

	shadow = 1.0 - shadow;
	return ambientColor + shadow * diffuseColor;
}

vec3 getSpotlightColor(
	Spotlight light,
	vec3 normal,
	vec3 position,
	vec3 albedoTextureColor)
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

	vec3 ambientColor = 0.1 * albedoTextureColor * attenuation * intensity;
	vec3 diffuseColor = light.radiantFlux * diffuseValue * albedoTextureColor *
		attenuation * intensity;

	float shadow = 0.0;
	if (light.shadowIndex > -1)
	{
		shadow = getDirectionalShadow(
			fragSpotlightSpacePositions[light.shadowIndex],
			normal,
			light.direction,
			shadowSpotlightBiasRange.x,
			shadowSpotlightBiasRange.y,
			spotlightShadowMaps[light.shadowIndex]);
	}

	shadow = 1.0 - shadow;
	return ambientColor + shadow * diffuseColor;
}

float getDirectionalShadow(
	vec4 lightSpacePosition,
	vec3 normal,
	vec3 lightDirection,
	float minBias,
	float maxBias,
	sampler2D shadowMap)
{
	vec3 projectedCoordinates = lightSpacePosition.xyz / lightSpacePosition.w;
	projectedCoordinates = projectedCoordinates * 0.5 + 0.5;

	float currentDepth = projectedCoordinates.z;
	if (currentDepth > 1.0)
	{
		return 0.0;
	}

	float bias = max(maxBias * (1.0 - dot(normal, -lightDirection)), minBias);
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			vec2 offset = vec2(x, y) * texelSize;
			float closestDepth = texture(
				shadowMap,
				projectedCoordinates.xy + offset).r;
			shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
		}
	}

	shadow /= 9.0;
	return shadow;
}

float getPointShadow(
	vec3 position,
	vec3 lightPosition,
	float farPlane,
	samplerCube shadowMap)
{
	vec3 lightSpacePosition = position - lightPosition;
	float currentDepth = length(lightSpacePosition);

	float shadow = 0.0;
	float bias = shadowPointLightBias;
	uint numSamples = 20;
	float cameraDistance = length(cameraPosition - position);
	float diskRadius = 1.0 + (cameraDistance / farPlane);
	diskRadius /= shadowPointLightDiskRadius;

	for (uint i = 0; i < numSamples; i++)
	{
		vec3 offset = pcfSampleOffsets[i] * diskRadius;
		float closestDepth = texture(
			shadowMap,
			lightSpacePosition + offset).r * farPlane;
		shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
	}

	shadow /= float(numSamples);
	return shadow;
}