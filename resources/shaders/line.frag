#version 420 core

in vec3 fragPosition;
in vec4 fragColor;

out vec4 color;

void main()
{
	color = fragColor;
}