#include "renderer/renderer_utilities.h"

#include "asset_management/material.h"
#include "asset_management/texture.h"

#include "core/log.h"

#include <GL/glu.h>

#include <stdarg.h>
#include <malloc.h>
#include <string.h>

int32 logGLError(bool logNoError, const char *message, ...)
{
	int32 error = 0;

	va_list args;
    va_start(args, message);

	char *messageBuffer = malloc(strlen(message) + 4096);
	vsprintf(messageBuffer, message, args);
    va_end(args);

	GLenum glError = glGetError();
	if (glError != GL_NO_ERROR)
	{
		error = -1;
	}

	bool logError = error == -1 || (error == 0 && logNoError);
	if (logError)
	{
		LOG("%s: %s", messageBuffer, gluErrorString(glError));
	}

	free(messageBuffer);

	do
	{
		glError = glGetError();
		if (glError != GL_NO_ERROR)
		{
			LOG(", %s", gluErrorString(glError));
		}
		else
		{
			break;
		}
	} while (true);

	if (logError)
	{
		LOG("\n");
	}

	return error;
}

int32 setMaterialUniform(Uniform *uniform, GLint *textureIndex)
{
	return setTextureArrayUniform(
		uniform,
		MATERIAL_COMPONENT_TYPE_COUNT,
		textureIndex);
}

int32 setMaterialValuesUniform(Uniform *uniform, Material *material)
{
	kmVec3 materialValues[MATERIAL_COMPONENT_TYPE_COUNT];
	for (uint8 i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
	{
		materialValues[i] = material->components[i].value;
	}

	if (setUniform(
		*uniform,
		MATERIAL_COMPONENT_TYPE_COUNT,
		materialValues) == -1)
	{
		return -1;
	}

	return 0;
}

int32 setTextureArrayUniform(
	Uniform *uniform,
	uint32 numTextures,
	GLint *textureIndex)
{
	GLint textureIndices[numTextures];
	for (uint8 i = 0; i < numTextures; i++)
	{
		textureIndices[i] = (*textureIndex)++;
	}

	if (setUniform(*uniform, numTextures, textureIndices) == -1)
	{
		return -1;
	}

	return 0;
}

void activateMaterialTextures(Material *material, GLint *textureIndex)
{
	for (uint8 i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
	{
		activateTexture(material->components[i].texture, textureIndex);
	}
}

void activateTextures(
	uint32 numTextures,
	GLenum type,
	GLuint *textures,
	GLint *textureIndex)
{
	for (uint8 i = 0; i < numTextures; i++)
	{
		glActiveTexture(GL_TEXTURE0 + (*textureIndex)++);
		glBindTexture(type, textures[i]);
	}
}

void activateTexture(UUID name, GLint *textureIndex)
{
	Texture texture = getTexture(name.string);
	if (strlen(texture.name.string) > 0)
	{
		glActiveTexture(GL_TEXTURE0 + *textureIndex);
		glBindTexture(GL_TEXTURE_2D, texture.id);
	}

	(*textureIndex)++;
}