#version 420 core

#define NUM_BONES 4
#define MAX_BONE_COUNT 128

layout(location=0) in vec3 position;
layout(location=1) in vec4 color;
layout(location=2) in vec3 normal;
layout(location=3) in vec3 tangent;
layout(location=4) in vec3 bitangent;
layout(location=5) in vec2 materialUV;
layout(location=6) in vec2 maskUV;
layout(location=7) in ivec4 bones;
layout(location=8) in vec4 weights;

uniform mat4 model;

uniform bool hasAnimations;
uniform mat4 boneTransforms[MAX_BONE_COUNT];

uniform mat4 lightTransform;

void main()
{
	mat4 boneTransform = mat4(hasAnimations ? 0.0 : 1.0);
	if (hasAnimations)
	{
		for (int i = 0; i < NUM_BONES; i++)
		{
			boneTransform += boneTransforms[bones[i]] * weights[i];
		}
	}

	gl_Position = lightTransform * model * boneTransform * vec4(position, 1);
}