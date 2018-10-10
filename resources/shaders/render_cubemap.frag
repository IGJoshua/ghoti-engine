#version 420 core

in vec3 fragPosition;

out vec4 color;

uniform samplerCube cubemapTexture;
uniform float cubemapMipLevel;

void main()
{
	vec3 environmentColor = cubemapMipLevel < 0.0 ?
		texture(cubemapTexture, fragPosition).rgb :
		textureLod(cubemapTexture, fragPosition, cubemapMipLevel).rgb;
	environmentColor /= (environmentColor + vec3(1.0));
	environmentColor = pow(environmentColor, vec3(1.0 / 2.2));

	color = vec4(environmentColor, 1.0);
}