#version 420 core

in vec2 fragUV;

out vec4 color;

uniform sampler2D screenTexture;

void main()
{
	vec3 finalColor = texture(screenTexture, fragUV).rgb;
	color = vec4(finalColor, 1.0);
}