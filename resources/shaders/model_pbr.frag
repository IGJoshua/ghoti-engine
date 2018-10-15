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
in vec3 fragTangent;
in vec2 fragMaterialUV;
in vec2 fragMaskUV;

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

mat3 createTBNMatrix(vec3 normal, vec3 tangent);
vec2 getMaterialUV(vec3 viewDirection, mat3 TBN);

vec4 getMask(vec2 uv);

float getAmbientOcclusion(vec2 uv, vec4 mask);
vec3 getAlbedo(vec2 uv, vec4 mask);
float getMetallic(vec2 uv, vec4 mask);
vec3 getNormal(vec2 uv, mat3 TBN, vec4 mask);
float getRoughness(vec2 uv, vec4 mask);

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
	mat3 TBN = createTBNMatrix(fragNormal, fragTangent);

	vec2 materialUV = getMaterialUV(viewDirection, TBN);
	vec4 mask = getMask(fragMaskUV);

	float ambientOcclusion = getAmbientOcclusion(materialUV, mask);
	vec3 albedo = getAlbedo(materialUV, mask);
	float metallic = getMetallic(materialUV, mask);
	vec3 normal = getNormal(materialUV, TBN, mask);
	float roughness = getRoughness(materialUV, mask);

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

mat3 createTBNMatrix(vec3 normal, vec3 tangent)
{
	tangent = normalize(tangent - dot(tangent, normal) * normal);
	vec3 bitangent = cross(tangent, normal);
	return mat3(tangent, bitangent, normal);
}

vec2 getMaterialUV(vec3 viewDirection, mat3 TBN)
{
	if (!materialActive[HEIGHT_COMPONENT])
	{
		return fragMaterialUV;
	}

	viewDirection = normalize(viewDirection * TBN);

	const vec2 layersRange = vec2(8, 32);
	float numLayers = mix(
		layersRange.y,
		layersRange.x,
		abs(dot(vec3(0.0, 0.0, 1.0), viewDirection)));

	vec2 P =
		(viewDirection.xy / viewDirection.z) *
		0.05 * materialValues[HEIGHT_COMPONENT].x;

	float layerDepth = 1.0 / numLayers;
	vec2 deltaUV = P / numLayers;

	sampler2D s = sampler2D(material[HEIGHT_COMPONENT]);
	float height =
		materialValues[HEIGHT_COMPONENT].y - texture(s, fragMaterialUV).r;

	vec2 uv; float l;
	for (uv = fragMaterialUV, l = 0.0; l < height; l += layerDepth)
	{
		uv -= deltaUV;
		height = materialValues[HEIGHT_COMPONENT].y - texture(s, uv).r;
	}

	vec2 lastUV = uv + deltaUV;

	vec2 depth = vec2(
		materialValues[HEIGHT_COMPONENT].y - texture(s, lastUV).r -
			l + layerDepth,
		height - l);
	float weight = depth.y / (depth.y - depth.x);

	vec2 materialUV = lastUV * weight + uv * (1.0 - weight);

	if (materialValues[HEIGHT_COMPONENT].z == -1.0)
	{
		if (materialUV.x < 0.0 ||
			materialUV.x > 1.0 ||
			materialUV.y < 0.0 ||
			materialUV.y > 1.0)
		{
			discard;
		}
	}

	return materialUV;
}

vec4 getMask(vec2 uv)
{
	sampler2D sampler = sampler2D(materialMask);
	vec4 mask = texture(sampler, uv);

	if (mask.r < 1.0)
	{
		mask.r = 0.0;
	}

	if (mask.g < 1.0)
	{
		mask.g = 0.0;
	}

	if (mask.b < 1.0)
	{
		mask.b = 0.0;
	}

	if (mask.rgb == vec3(0.0))
	{
		mask.a = 0.0;
	}

	return mask;
}

float getAmbientOcclusion(vec2 uv, vec4 mask)
{
	float ambientOcclusion = 0.0;
	if (materialActive[AMBIENT_OCCLUSION_COMPONENT])
	{
		sampler2D sampler = sampler2D(material[AMBIENT_OCCLUSION_COMPONENT]);
		ambientOcclusion = texture(sampler, uv).r;
	}

	sampler2D wearSampler = sampler2D(
		wearMaterial[AMBIENT_OCCLUSION_COMPONENT]);
	sampler2D grungeSampler = sampler2D(
		grungeMaterial[AMBIENT_OCCLUSION_COMPONENT]);
	sampler2D collectionSampler = sampler2D(
		collectionMaterial[AMBIENT_OCCLUSION_COMPONENT]);

	float maskAmbientOcclusion =
		texture(wearSampler, uv).r * mask.r +
		texture(grungeSampler, uv).r * mask.g +
		texture(collectionSampler, uv).r * mask.b;

	if (maskAmbientOcclusion == 0.0)
	{
		mask.a = 0.0;
	}

	ambientOcclusion = mix(ambientOcclusion, maskAmbientOcclusion, mask.a);
	ambientOcclusion *= materialValues[AMBIENT_OCCLUSION_COMPONENT].x;

	return ambientOcclusion;
}

vec3 getAlbedo(vec2 uv, vec4 mask)
{
	vec3 albedo = fragColor.rgb;
	if (materialActive[BASE_COMPONENT])
	{
		sampler2D sampler = sampler2D(material[BASE_COMPONENT]);
		albedo = texture(sampler, uv).rgb;
	}

	sampler2D wearSampler = sampler2D(wearMaterial[BASE_COMPONENT]);
	sampler2D grungeSampler = sampler2D(grungeMaterial[BASE_COMPONENT]);
	sampler2D collectionSampler = sampler2D(
		collectionMaterial[BASE_COMPONENT]);

	vec3 maskAlbedo =
		texture(wearSampler, uv).rgb * mask.r +
		texture(grungeSampler, uv).rgb * mask.g +
		texture(collectionSampler, uv).rgb * mask.b;

	if (maskAlbedo == vec3(0.0))
	{
		mask.a = 0.0;
	}

	albedo = mix(albedo, maskAlbedo, mask.a);
	albedo = pow(albedo * materialValues[BASE_COMPONENT], vec3(2.2));

	return albedo;
}

float getMetallic(vec2 uv, vec4 mask)
{
	float metallic = 0.0;
	if (materialActive[METALLIC_COMPONENT])
	{
		sampler2D sampler = sampler2D(material[METALLIC_COMPONENT]);
		metallic = texture(sampler, uv).r;
	}

	sampler2D wearSampler = sampler2D(wearMaterial[METALLIC_COMPONENT]);
	sampler2D grungeSampler = sampler2D(grungeMaterial[METALLIC_COMPONENT]);
	sampler2D collectionSampler = sampler2D(
		collectionMaterial[METALLIC_COMPONENT]);

	float maskMetallic =
		texture(wearSampler, uv).r * mask.r +
		texture(grungeSampler, uv).r * mask.g +
		texture(collectionSampler, uv).r * mask.b;

	if (maskMetallic == 0.0)
	{
		mask.a = 0.0;
	}

	metallic = mix(metallic, maskMetallic, mask.a);
	metallic *= materialValues[METALLIC_COMPONENT].x;

	return metallic;
}

vec3 getNormal(vec2 uv, mat3 TBN, vec4 mask)
{
	vec3 normal = fragNormal;
	if (materialActive[NORMAL_COMPONENT])
	{
		sampler2D sampler = sampler2D(material[NORMAL_COMPONENT]);
		normal = texture(sampler, uv).rgb;
	}

	sampler2D wearSampler = sampler2D(wearMaterial[NORMAL_COMPONENT]);
	sampler2D grungeSampler = sampler2D(grungeMaterial[NORMAL_COMPONENT]);
	sampler2D collectionSampler = sampler2D(
		collectionMaterial[NORMAL_COMPONENT]);

	vec3 maskNormal =
		texture(wearSampler, uv).rgb * mask.r +
		texture(grungeSampler, uv).rgb * mask.g +
		texture(collectionSampler, uv).rgb * mask.b;

	if (maskNormal == vec3(0.0))
	{
		mask.a = 0.0;
	}

	normal = mix(normal, maskNormal, mask.a);
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(vec3(
		normal.rg * materialValues[NORMAL_COMPONENT].x,
		normal.b));
	normal = normalize(TBN * normal);

	return normal;
}

float getRoughness(vec2 uv, vec4 mask)
{
	float roughness = 0.0;
	if (materialActive[ROUGHNESS_COMPONENT])
	{
		sampler2D sampler = sampler2D(material[ROUGHNESS_COMPONENT]);
		roughness = texture(sampler, uv).r;
	}

	sampler2D wearSampler = sampler2D(wearMaterial[ROUGHNESS_COMPONENT]);
	sampler2D grungeSampler = sampler2D(
		grungeMaterial[ROUGHNESS_COMPONENT]);
	sampler2D collectionSampler = sampler2D(
		collectionMaterial[ROUGHNESS_COMPONENT]);

	float maskRoughness =
		texture(wearSampler, uv).r * mask.r +
		texture(grungeSampler, uv).r * mask.g +
		texture(collectionSampler, uv).r * mask.b;

	if (maskRoughness == 0.0)
	{
		mask.a = 0.0;
	}

	roughness = mix(roughness, maskRoughness, mask.a);
	roughness *= materialValues[ROUGHNESS_COMPONENT].x;

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