#version 420 core

layout(location=0) in vec3 position;
layout(location=1) in vec2 size;
layout(location=2) in vec2 uv;
layout(location=3) in vec2 spriteSize;
layout(location=4) in vec4 color;
layout(location=5) in int texture;

out vec2 geomSize;
out vec2 geomUV;
out vec2 geomSpriteSize;
out vec4 geomColor;
flat out int geomTexture;

uniform mat4 view;

void main()
{
	geomSize = size;
	geomUV = uv;
	geomSpriteSize = spriteSize;
	geomColor = color;
	geomTexture = texture;

	gl_Position = view * vec4(position, 1.0);
}