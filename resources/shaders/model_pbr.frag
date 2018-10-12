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

const float PI = 3.14159265359;

#define MAX_NUM_POINT_LIGHTS 64
#define MAX_NUM_SPOTLIGHTS 64

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
in vec2 fragMaterialUV;
in vec2 fragMaskUV;
in mat3 fragTBN;

out vec4 color;

uniform vec3 cameraPosition;

uniform bool materialActive[NUM_MATERIAL_COMPONENTS];
uniform uint64_t material[NUM_MATERIAL_COMPONENTS];
uniform vec3 materialValues[NUM_MATERIAL_COMPONENTS];
uniform uint64_t materialMask;
uniform uint64_t opacityMask;
uniform uint64_t collectionMaterial[NUM_MATERIAL_COMPONENTS];
uniform vec3 collectionMaterialValues[NUM_MATERIAL_COMPONENTS];
uniform uint64_t grungeMaterial[NUM_MATERIAL_COMPONENTS];
uniform vec3 grungeMaterialValues[NUM_MATERIAL_COMPONENTS];
uniform uint64_t wearMaterial[NUM_MATERIAL_COMPONENTS];
uniform vec3 wearMaterialValues[NUM_MATERIAL_COMPONENTS];

uniform bool useCustomColor;
uniform vec3 customColor;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

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

float squared(float n);

vec2 getMaterialUV(vec3 viewDirection);

float getAmbientOcclusion(vec2 uv);
vec3 getAlbedo(vec2 uv);
float getMetallic(vec2 uv);
vec3 getNormal(vec2 uv);
float getRoughness(vec2 uv);

vec3 getDirectionalLightRadiance(
	DirectionalLight light,
	vec3 viewDirection,
	vec3 albedo,
	float metallic,
	vec3 normal,
	float roughness,
	vec3 F0);
vec3 getPointLightRadiance(
	PointLight light,
	vec3 position,
	vec3 viewDirection,
	vec3 albedo,
	float metallic,
	vec3 normal,
	float roughness,
	vec3 F0);
vec3 getSpotlightRadiance(
	Spotlight light,
	vec3 position,
	vec3 viewDirection,
	vec3 albedo,
	float metallic,
	vec3 normal,
	float roughness,
	vec3 F0);

float normalDistribution(vec3 normal, vec3 halfwayDirection, float roughness);
vec3 fresnelEquation(vec3 halfwayDirection, vec3 viewDirection, vec3 F0);
vec3 fresnelEquationRoughness(
	vec3 normal,
	vec3 viewDirection,
	vec3 F0,
	float roughness);
float geometrySubFunction(float NdotV, float roughness);
float geometryFunction(
	vec3 normal,
	vec3 viewDirection,
	vec3 lightDirection,
	float roughness);

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

	float ambientOcclusion = getAmbientOcclusion(materialUV);
	vec3 albedo = getAlbedo(materialUV);
	float metallic = getMetallic(materialUV);
	vec3 normal = getNormal(materialUV);
	float roughness = getRoughness(materialUV);

	// F0 - Reflectance when looking directly at the surface
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	vec3 radiance = vec3(0.0);

	if (numDirectionalLights == 1)
	{
		radiance += getDirectionalLightRadiance(
			directionalLight,
			viewDirection,
			albedo,
			metallic,
			normal,
			roughness,
			F0);
	}

	for (uint i = 0; i < numPointLights; i++)
	{
		radiance += getPointLightRadiance(
			pointLights[i],
			fragPosition,
			viewDirection,
			albedo,
			metallic,
			normal,
			roughness,
			F0);
	}

	for (uint i = 0; i < numSpotlights; i++)
	{
		radiance += getSpotlightRadiance(
			spotlights[i],
			fragPosition,
			viewDirection,
			albedo,
			metallic,
			normal,
			roughness,
			F0);
	}

	vec3 F = fresnelEquationRoughness(normal, viewDirection, F0, roughness);

	vec3 kS = F;
	vec3 kD = (1.0 - kS) * (1.0 - metallic);

	vec3 irradiance = texture(irradianceMap, normal).rgb;

	vec3 reflectionDirection = reflect(-viewDirection, normal);
	const float maxReflectionLOD = 4.0;
	vec3 prefilteredRadiance = textureLod(
		prefilterMap,
		reflectionDirection,
		roughness * maxReflectionLOD).rgb;

	vec2 brdf = texture(
		brdfLUT,
		vec2(max(dot(normal, viewDirection), 0.0), roughness)).rg;

	vec3 specular = prefilteredRadiance * (F * brdf.x + brdf.y);

	vec3 ambient = (kD * irradiance * albedo + specular) * ambientOcclusion;
	vec3 finalColor = ambient + radiance;

	// Reinhard's Tone-Mapping Operator
	finalColor /= (finalColor + vec3(1.0));
	finalColor = pow(finalColor, vec3(1.0 / 2.2));

	color = vec4(finalColor, 1.0);
}

float squared(float n)
{
	return n * n;
}

vec2 getMaterialUV(vec3 viewDirection)
{
	if (!materialActive[HEIGHT_COMPONENT])
	{
		return fragMaterialUV;
	}

	return fragMaterialUV;
}

float getAmbientOcclusion(vec2 uv)
{
	float ambientOcclusion = 0.0;
	if (materialActive[AMBIENT_OCCLUSION_COMPONENT])
	{
		sampler2D sampler = sampler2D(material[AMBIENT_OCCLUSION_COMPONENT]);
		ambientOcclusion = texture(sampler, uv).r;
		ambientOcclusion *= materialValues[AMBIENT_OCCLUSION_COMPONENT].x;
	}

	return ambientOcclusion;
}

vec3 getAlbedo(vec2 uv)
{
	vec3 albedo = fragColor.rgb;
	if (materialActive[BASE_COMPONENT])
	{
		sampler2D sampler = sampler2D(material[BASE_COMPONENT]);
		albedo = pow(texture(sampler, uv).rgb, vec3(2.2));
		albedo *= materialValues[BASE_COMPONENT];
	}

	return albedo;
}

float getMetallic(vec2 uv)
{
	float metallic = 0.0;
	if (materialActive[METALLIC_COMPONENT])
	{
		sampler2D sampler = sampler2D(material[METALLIC_COMPONENT]);
		metallic = texture(sampler, uv).r;
		metallic *= materialValues[METALLIC_COMPONENT].x;
	}

	return metallic;
}

vec3 getNormal(vec2 uv)
{
	vec3 normal = fragNormal;
	// if (materialActive[NORMAL_COMPONENT])
	// {
	// 	sampler2D sampler = sampler2D(material[NORMAL_COMPONENT]);
	// 	normal = texture(sampler, uv).rgb;
	// 	normal = normalize(normal * 2.0 - 1.0);
	// 	normal = normalize(fragTBN * normal);
	// }

	return normal;
}

float getRoughness(vec2 uv)
{
	float roughness = 0.0;
	if (materialActive[ROUGHNESS_COMPONENT])
	{
		sampler2D sampler = sampler2D(material[ROUGHNESS_COMPONENT]);
		roughness = texture(sampler, uv).r;
		roughness *= materialValues[ROUGHNESS_COMPONENT].x;
	}

	return roughness;
}

vec3 getDirectionalLightRadiance(
	DirectionalLight light,
	vec3 viewDirection,
	vec3 albedo,
	float metallic,
	vec3 normal,
	float roughness,
	vec3 F0)
{
	vec3 lightDirection = normalize(-light.direction);
	vec3 halfwayDirection = normalize(viewDirection + lightDirection);

	vec3 radiance = light.radiantFlux;

	// Cook-Torrance BRDF
	float D = normalDistribution(normal, halfwayDirection, roughness);
	vec3 F = fresnelEquation(halfwayDirection, viewDirection, F0);
	float G = geometryFunction(
		normal,
		viewDirection,
		lightDirection,
		roughness);

	vec3 specular = D * F * G;
	specular /= (4 *
		max(dot(normal, viewDirection), 0.0) *
		max(dot(normal, lightDirection), 0.0) + 0.001);

	// kS - Reflectance ratio
	vec3 kS = F;
	// kD - Diffuse component
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

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

	return (kD * (albedo / PI) + specular) * radiance * shadow *
		max(dot(normal, lightDirection), 0.0);
}

vec3 getPointLightRadiance(
	PointLight light,
	vec3 position,
	vec3 viewDirection,
	vec3 albedo,
	float metallic,
	vec3 normal,
	float roughness,
	vec3 F0)
{
	vec3 lightDirection = normalize(light.position - position);
	vec3 halfwayDirection = normalize(viewDirection + lightDirection);

	float distance = length(light.position - position);
	float attenuation = clamp(
		1.0 - (squared(distance) / squared(light.radius)),
		0.0,
		1.0);
	attenuation *= attenuation;

	vec3 radiance = light.radiantFlux * attenuation;

	// Cook-Torrance BRDF
	float D = normalDistribution(normal, halfwayDirection, roughness);
	vec3 F = fresnelEquation(halfwayDirection, viewDirection, F0);
	float G = geometryFunction(
		normal,
		viewDirection,
		lightDirection,
		roughness);

	vec3 specular = D * F * G;
	specular /= (4 *
		max(dot(normal, viewDirection), 0.0) *
		max(dot(normal, lightDirection), 0.0) + 0.001);

	// kS - Reflectance ratio
	vec3 kS = F;
	// kD - Diffuse component
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

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

	return (kD * (albedo / PI) + specular) * radiance * shadow *
		max(dot(normal, lightDirection), 0.0);
}

vec3 getSpotlightRadiance(
	Spotlight light,
	vec3 position,
	vec3 viewDirection,
	vec3 albedo,
	float metallic,
	vec3 normal,
	float roughness,
	vec3 F0)
{
	vec3 lightDirection = normalize(light.position - position);
	vec3 halfwayDirection = normalize(viewDirection + lightDirection);

	float distance = length(light.position - position);
	float attenuation = clamp(
		1.0 - (squared(distance) / squared(light.radius)),
		0.0,
		1.0);
	attenuation *= attenuation;

	float theta = dot(lightDirection, normalize(-light.direction));
	float epsilon = light.size.x - light.size.y;
	float intensity = clamp((theta - light.size.y) / epsilon, 0.0, 1.0);

	vec3 radiance = light.radiantFlux * attenuation * intensity;

	// Cook-Torrance BRDF
	float D = normalDistribution(normal, halfwayDirection, roughness);
	vec3 F = fresnelEquation(halfwayDirection, viewDirection, F0);
	float G = geometryFunction(
		normal,
		viewDirection,
		lightDirection,
		roughness);

	vec3 specular = D * F * G;
	specular /= (4 *
		max(dot(normal, viewDirection), 0.0) *
		max(dot(normal, lightDirection), 0.0) + 0.001);

	// kS - Reflectance ratio
	vec3 kS = F;
	// kD - Diffuse component
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

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

	return (kD * (albedo / PI) + specular) * radiance * shadow *
		max(dot(normal, lightDirection), 0.0);
}

float normalDistribution(vec3 normal, vec3 halfwayDirection, float roughness)
{
	// Trowbridge-Reitz GGX Distribution
	float a2 = squared(squared(roughness));
	float denominator = squared(max(dot(normal, halfwayDirection), 0.0));
	denominator *= (a2 - 1.0);
	denominator = PI * squared(denominator + 1.0);
	return a2 / denominator;
}

vec3 fresnelEquation(vec3 halfwayDirection, vec3 viewDirection, vec3 F0)
{
	// Fresnel-Schlick approximation
	float cosTheta = max(dot(halfwayDirection, viewDirection), 0.0);
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelEquationRoughness(
	vec3 normal,
	vec3 viewDirection,
	vec3 F0,
	float roughness)
{
	// Sébastien Lagarde's roughness-based Fresnel-Schlick approximation
	float cosTheta = max(dot(normal, viewDirection), 0.0);
	return
		F0 +
		(max(vec3(1.0 - roughness), F0) - F0) *
		pow(1.0 - cosTheta, 5.0);
}

float geometrySubFunction(float NdotV, float roughness)
{
	// Schlick-Beckmann approximation (Schlick-GGX)
	float k = squared(roughness + 1.0) / 8.0;
	return NdotV / (NdotV * (1.0 - k) + k);
}

float geometryFunction(
	vec3 normal,
	vec3 viewDirection,
	vec3 lightDirection,
	float roughness)
{
	// Smith's Schlick-GGX
	return
		// Geometry obstruction
		geometrySubFunction(max(dot(normal, viewDirection), 0.0), roughness) *
		// Geometry shadowing
		geometrySubFunction(max(dot(normal, lightDirection), 0.0), roughness);
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