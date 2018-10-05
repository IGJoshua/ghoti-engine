#version 420 core

in vec2 fragUV;

out vec4 color;

uniform sampler2D screenTexture;

void main()
{
	vec3 fragColor = texture(screenTexture, fragUV).rgb;
	float average =
		0.2126 * fragColor.r +
		0.7152 * fragColor.g +
		0.0722 * fragColor.b;
	color = vec4(average, average, average, 1.0);
}