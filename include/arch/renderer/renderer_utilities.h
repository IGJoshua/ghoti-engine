#pragma once
#include "defines.h"

#include "renderer/shader.h"
#include "renderer/renderer_types.h"

#include <GL/glew.h>

int32 logGLError(bool logNoError, const char *message, ...);

int32 setMaterialUniform(Uniform *uniform, GLint *textureIndex);
int32 setMaterialValuesUniform(Uniform *uniform, Material *material);
int32 setTextureArrayUniform(
	Uniform *uniform,
	uint32 numTextures,
	GLint *textureIndex);

void activateMaterialTextures(Material *material, GLint *textureIndex);
void activateTextures(
	uint32 numTextures,
	GLuint *textures,
	GLint *textureIndex);
void activateTexture(UUID name, GLint *textureIndex);