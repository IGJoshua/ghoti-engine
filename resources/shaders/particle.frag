#version 420 core

#define MAX_PARTICLE_EMITTER_TEXTURE_COUNT 32

in vec2 fragUV;
in vec4 fragColor;
flat in int fragTexture;

out vec4 color;

uniform sampler2D textures[MAX_PARTICLE_EMITTER_TEXTURE_COUNT];

void main()
{
	color = fragColor;

	if (fragTexture > -1)
	{
		color *= texture(textures[fragTexture], fragUV);
	}
}