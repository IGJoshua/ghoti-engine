#version 420 core

layout(location=0) in vec3 position;
layout(location=1) in vec2 size;
layout(location=2) in vec4 color;
layout(location=3) in int texture;

out vec2 geomSize;
out vec4 geomColor;
flat out int geomTexture;

uniform mat4 view;

void main()
{
	geomSize = size;
	geomColor = color;
	geomTexture = texture;

	gl_Position = view * vec4(position, 1);
}