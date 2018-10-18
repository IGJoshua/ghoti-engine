#version 420 core

in vec3 fragPosition;

out vec4 color;

uniform samplerCube cubemapTexture;

void main()
{
	vec3 environmentColor = texture(cubemapTexture, fragPosition).rgb;
	environmentColor /= (environmentColor + vec3(1.0));
	environmentColor = pow(environmentColor, vec3(1.0 / 2.2));

	color = vec4(environmentColor, 1.0);
}