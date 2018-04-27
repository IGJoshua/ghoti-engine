#version 420 core
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragUV;

out vec4 color;

const vec3 lightDirection = normalize(vec3(0.25, -0.5, -0.25));

uniform sampler2D diffuseTexture;

void main()
{
	float normalAttenuation = clamp(dot(-lightDirection, fragNormal), 0, 1);

	vec4 diffuseColor = texture(diffuseTexture, fragUV);
	color = diffuseColor * normalAttenuation * vec4(1, 1, 1, 1);
}
