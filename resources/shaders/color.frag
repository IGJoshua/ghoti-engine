#version 420 core

in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragDiffuseUV;
in vec2 fragSpecularUV;
in vec2 fragNormalUV;
in vec2 fragEmissiveUV;

out vec4 color;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;
uniform sampler2D emissiveTexture;

const vec3 lightDirection = normalize(vec3(0.25, -0.5, -0.25));

void main()
{
	float normalAttenuation = clamp(dot(-lightDirection, fragNormal), 0, 1);

	vec4 diffuseColor = texture(diffuseTexture, fragDiffuseUV);
	color = diffuseColor * normalAttenuation * vec4(1, 1, 1, 1);
}
