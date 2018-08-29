#version 420 core

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
out vec3 fragNormal;
out vec2 fragMaterialUV;
out vec2 fragMaskUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	fragColor = color;
	fragPosition = (model * vec4(position, 1)).xyz;
	fragNormal = normalize((model * vec4(normal, 0)).xyz);
	fragMaterialUV = materialUV;
	fragMaskUV = maskUV;

	gl_Position = projection * view * vec4(fragPosition, 1);
}