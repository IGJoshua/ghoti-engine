#version 420 core

in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragDiffuseUV;
in vec2 fragSpecularUV;
in vec2 fragNormalUV;
in vec2 fragEmissiveUV;

out vec4 color;

void main()
{
	color = fragColor;
}
