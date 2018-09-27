#version 420 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 color;

out vec4 fragColor;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	fragColor = vec4(color, 1.0);

	gl_Position = projection * view * vec4(position, 1);
}
