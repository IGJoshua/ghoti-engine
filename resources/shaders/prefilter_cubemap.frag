#version 420 core

const float PI = 3.14159265359;

in vec3 fragPosition;

out vec4 color;

uniform samplerCube cubemapTexture;
uniform float cubemapResolution;

uniform float roughness;

float squared(float n);

float radicalInverse(uint bits);
vec2 lowDiscrepancySample(uint i, uint sampleSetSize);
vec3 importanceSampleNormalDistribution(vec2 Xi, vec3 normal, float roughness);
float normalDistribution(vec3 normal, vec3 halfwayDirection, float roughness);

void main()
{
	vec3 normal = normalize(fragPosition);

	vec3 specularReflectionDirection = normal;
	vec3 viewDirection = specularReflectionDirection;

	vec3 prefilteredColor = vec3(0.0);
	float weight = 0.0;

	const uint numSamples = 1024u;
	for (uint i = 0u; i < numSamples; i++)
	{
		// Importance Sampling
		// Xi - Low-Discrepancy Sequence Value
		vec2 Xi = lowDiscrepancySample(i, numSamples);
		vec3 halfwayDirection = importanceSampleNormalDistribution(
			Xi,
			normal,
			roughness);
		vec3 lightDirection = normalize(
			2.0 * dot(viewDirection, halfwayDirection) * halfwayDirection -
			viewDirection);

		float NdotL = max(dot(normal, lightDirection), 0.0);
		if (NdotL > 0.0)
		{
			float D = normalDistribution(normal, halfwayDirection, roughness);
			// pdf - Probability Density Function
			float pdf =
				((D * max(dot(normal, halfwayDirection), 0.0)) /
				(4.0 * max(dot(halfwayDirection, viewDirection), 0.0)))
				+ 0.0001;

			// Mip level selection based on roughness and pdf by Chetan Jags
			float mipLevel = (roughness == 0.0 ? 0.0 : 0.5) *
				log2((1.0 / (float(numSamples) * pdf + 0.0001)) /
					((4.0 * PI) / (6.0 * squared(cubemapResolution))));

			prefilteredColor += NdotL * textureLod(
				cubemapTexture,
				lightDirection,
				mipLevel).rgb;
			weight += NdotL;
		}
	}

	prefilteredColor /= weight;
	color = vec4(prefilteredColor, 1.0);
}

float squared(float n)
{
	return n * n;
}

float radicalInverse(uint bits)
{
	// Van der Corput sequence
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);

	// 2.3283064365386963e-10 = 0x100000000
	return float(bits) * 2.3283064365386963e-10;
}

vec2 lowDiscrepancySample(uint i, uint sampleSetSize)
{
	// Hammersley Sequence by Holger Dammertz
	return vec2(float(i) / float(sampleSetSize), radicalInverse(i));
}

vec3 importanceSampleNormalDistribution(vec2 Xi, vec3 normal, float roughness)
{
	// Trowbridge-Reitz GGX Distribution with importance sampling
	float a = squared(roughness);

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (squared(a) - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - squared(cosTheta));

	vec3 halfwayDirection = vec3(
		cos(phi) * sinTheta,
		sin(phi) * sinTheta,
		cosTheta);

	vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, normal));
	vec3 bitangent = cross(normal, tangent);

	vec3 sampleDirection =
		tangent * halfwayDirection.x +
		bitangent * halfwayDirection.y +
		normal * halfwayDirection.z;

	return normalize(sampleDirection);
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