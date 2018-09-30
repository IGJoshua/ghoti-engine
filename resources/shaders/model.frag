#version 420 core

#define NUM_MATERIAL_COMPONENTS 5

const uint BASE_COMPONENT = 0;
const uint EMISSIVE_COMPONENT = 1;
const uint METALLIC_COMPONENT = 2;
const uint NORMAL_COMPONENT = 3;
const uint ROUGHNESS_COMPONENT = 4;

#define MAX_NUM_POINT_LIGHTS 256
#define MAX_NUM_SPOTLIGHTS 256

in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragMaterialUV;
in vec2 fragMaskUV;

out vec4 color;

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
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
};

struct Spotlight
{
	vec3 color;
	vec3 ambient;
	vec3 position;
	vec3 direction;
	vec2 size;
};

uniform uint numDirectionalLights;
uniform DirectionalLight directionalLight;

uniform uint numPointLights;
uniform PointLight pointLights[MAX_NUM_POINT_LIGHTS];

uniform uint numSpotlights;
uniform Spotlight spotlights[MAX_NUM_SPOTLIGHTS];

const vec3 lightDirection = normalize(vec3(0.25, -0.5, -0.25));

void main()
{
	if (useCustomColor)
	{
		color = vec4(customColor, 1);
		return;
	}

	float normalAttenuation = clamp(dot(-lightDirection, fragNormal), 0, 1);
	vec4 diffuseColor = texture(material[BASE_COMPONENT], fragMaterialUV);
	color = diffuseColor * normalAttenuation;
}