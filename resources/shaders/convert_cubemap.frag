#version 420 core

const vec2 inverseArcTangent = vec2(0.1591, 0.3183);

in vec3 fragPosition;

out vec4 color;

uniform sampler2D equirectangularCubemapTexture;

void main()
{
	vec3 direction = normalize(fragPosition);
	vec2 uv = vec2(atan(direction.z, direction.x), asin(direction.y));
	uv = uv * inverseArcTangent + 0.5;

	color = vec4(texture(equirectangularCubemapTexture, uv).rgb, 1.0);
}