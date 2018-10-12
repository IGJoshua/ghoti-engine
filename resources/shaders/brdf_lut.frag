#version 420 core

const float PI = 3.14159265359;

in vec2 fragUV;

out vec2 color;

float squared(float n);

float radicalInverse(uint bits);
vec2 lowDiscrepancySample(uint i, uint sampleSetSize);
vec3 importanceSampleNormalDistribution(vec2 Xi, vec3 normal, float roughness);
float geometrySubFunction(float NdotV, float roughness);
float geometryFunction(
	vec3 normal,
	vec3 viewDirection,
	vec3 lightDirection,
	float roughness);

void main()
{
	// Split Sum BRDF approximation
	float NdotV = fragUV.x;
	float roughness = fragUV.y;

	vec3 viewDirection = vec3(
		sqrt(1.0 - squared(NdotV)),
		0.0,
		NdotV);

	float scale = 0.0;
	float bias = 0.0;

	vec3 normal = vec3(0.0, 0.0, 1.0);

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

		if(max(lightDirection.z, 0.0) > 0.0)
		{
			float G = geometryFunction(
				normal,
				viewDirection,
				lightDirection,
				roughness);

			float VdotH = max(dot(viewDirection, halfwayDirection), 0.0);

			float G_Vis = (G * VdotH) / (max(halfwayDirection.z, 0.0) * NdotV);
			float Fc = pow(1.0 - VdotH, 5.0);

			scale += (1.0 - Fc) * G_Vis;
			bias += Fc * G_Vis;
		}
	}

	scale /= float(numSamples);
	bias /= float(numSamples);

	color = vec2(scale, bias);
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

float geometrySubFunction(float NdotV, float roughness)
{
	// Schlick-Beckmann approximation (Schlick-GGX)
	// IBL k value
	float k = squared(roughness) / 2.0;
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