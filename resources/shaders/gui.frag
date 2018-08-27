#version 420 core

in vec2 fragPosition;
in vec2 fragUV;
in vec4 fragColor;

out vec4 color;

uniform sampler2D font;

void main()
{
	color = fragColor * texture(font, fragUV.st);
}