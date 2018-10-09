#version 420 core

#define NUM_BONES 4
#define MAX_BONE_COUNT 128

#define MAX_NUM_SHADOW_SPOTLIGHTS 4

layout(location=0) in vec3 position;
layout(location=1) in vec4 color;
layout(location=2) in vec3 normal;
layout(location=3) in vec3 tangent;
layout(location=4) in vec3 bitangent;
layout(location=5) in vec2 materialUV;
layout(location=6) in vec2 maskUV;
layout(location=7) in ivec4 bones;
layout(location=8) in vec4 weights;

out vec4 fragColor;
out vec3 fragPosition;
out vec4 fragDirectionalLightSpacePosition;
out vec4 fragSpotlightSpacePositions[MAX_NUM_SHADOW_SPOTLIGHTS];
out vec3 fragNormal;
out vec2 fragMaterialUV;
out vec2 fragMaskUV;
out mat3 fragTBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool hasAnimations;
uniform mat4 boneTransforms[MAX_BONE_COUNT];

uniform mat4 shadowDirectionalLightTransform;
uniform mat4 shadowSpotlightTransforms[MAX_NUM_SHADOW_SPOTLIGHTS];

mat4 getBoneTransform(void);
mat3 createTBNMatrix(mat4 modelTransform);

void main()
{
	mat4 boneTransform = getBoneTransform();
	mat4 modelTransform = model * boneTransform;

	fragColor = color;
	fragPosition = (modelTransform * vec4(position, 1.0)).xyz;
	fragDirectionalLightSpacePosition =
		shadowDirectionalLightTransform * vec4(fragPosition, 1.0);

	for (uint i = 0; i < MAX_NUM_SHADOW_SPOTLIGHTS; i++)
	{
		fragSpotlightSpacePositions[i] =
			shadowSpotlightTransforms[i] * vec4(fragPosition, 1.0);
	}

	fragNormal = normalize((modelTransform * vec4(normal, 0.0)).xyz);
	fragMaterialUV = materialUV;
	fragMaskUV = maskUV;
	fragTBN = createTBNMatrix(modelTransform);

	gl_Position = projection * view * vec4(fragPosition, 1.0);
}

mat4 getBoneTransform(void)
{
	mat4 boneTransform = mat4(hasAnimations ? 0.0 : 1.0);
	if (hasAnimations)
	{
		for (int i = 0; i < NUM_BONES; i++)
		{
			boneTransform += boneTransforms[bones[i]] * weights[i];
		}
	}

	return boneTransform;
}

mat3 createTBNMatrix(mat4 modelTransform)
{
	mat3 normalTransform = transpose(inverse(mat3(modelTransform)));
	vec3 T = normalize(normalTransform * tangent);
	vec3 N = normalize(normalTransform * normal);
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	return transpose(mat3(T, B, N));
}