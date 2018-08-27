#version 420 core

layout(location=0) in vec2 position;
layout(location=1) in vec2 uv;
layout(location=2) in vec4 color;

out vec2 fragPosition;
out vec2 fragUV;
out vec4 fragColor;

uniform mat4 projection;

void main()
{
	fragPosition = position;
	fragUV = uv;
	fragColor = color;

	gl_Position = projection * vec4(fragPosition, 0, 1);
}