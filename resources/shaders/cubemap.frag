#version 420 core

in vec3 fragUV;

out vec4 color;

uniform samplerCube cubemapTexture;

void main()
{
	color = texture(cubemapTexture, fragUV);
}