#pragma once
#include "defines.h"

#include <GL/glew.h>

typedef enum texture_type_e
{
	TEXTURE_TYPE_DIFFUSE,
	TEXTURE_TYPE_SPECULAR,
	TEXTURE_TYPE_NORMAL,
	TEXTURE_TYPE_EMISSIVE,
	TEXTURE_TYPE_COUNT
} TextureType;

typedef struct texture_t
{
	TextureType type;
	char *name;
	GLuint id;
} Texture;