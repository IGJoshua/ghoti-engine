#include "asset_management/texture.h"

#include "file/utilities.h"

#include <IL/il.h>
#include <IL/ilu.h>

#include <malloc.h>
#include <string.h>

extern Texture *textures;
extern uint32 numTextures;
extern uint32 texturesCapacity;

int32 loadTexture(const char *name)
{
	Texture *texture = getTexture(name);

	if (name && !texture)
	{
		printf("Loading texture (%s)...\n", name);

		texture = &textures[numTextures++];

		texture->name = malloc(strlen(name) + 1);
		strcpy(texture->name, name);

		ILuint devilID;
		ilGenImages(1, &devilID);
		ilBindImage(devilID);

		char *filename = getFullFilePath(name, NULL, "resources/textures");
		ilLoadImage(filename);

		ILenum ilError = ilGetError();
		printf("Load %s: %s\n", filename, iluErrorString(ilError));
		if (ilError != IL_NO_ERROR)
		{
			free(filename);
			return -1;
		}

		free(filename);

		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

		glGenTextures(1, &texture->id);
		glBindTexture(GL_TEXTURE_2D, texture->id);

		GLsizei textureWidth = ilGetInteger(IL_IMAGE_WIDTH);
		GLsizei textureHeight = ilGetInteger(IL_IMAGE_HEIGHT);
		const GLvoid *textureData = ilGetData();
		glTexStorage2D(
			GL_TEXTURE_2D,
			1,
			GL_RGBA8,
			textureWidth,
			textureHeight);
		glTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			0,
			0,
			textureWidth,
			textureHeight,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			textureData);
		GLenum glError = glGetError();
		printf(
			"Load texture onto GPU: %s\n",
			gluErrorString(glError));
		if (glError != GL_NO_ERROR)
		{
			return -1;
		}

		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, 0);

		ilDeleteImages(1, &devilID);

		printf("Successfully loaded texture (%s)\n", name);
	}

	if (texture)
	{
		printf(
			"Texture (%s) Reference Count: %d\n",
			name,
			++(texture->refCount));
	}

	return 0;
}

void increaseTexturesCapacity(uint32 amount)
{
	while (numTextures + amount > texturesCapacity)
	{
		texturesCapacity += RESOURCE_REALLOCATION_AMOUNT;
	}

	uint32 previousBufferSize = numTextures * sizeof(Texture);
	uint32 newBufferSize = texturesCapacity * sizeof(Texture);

	if (previousBufferSize == 0)
	{
		textures = calloc(newBufferSize, 1);
	}
	else
	{
		textures = realloc(textures, newBufferSize);
		memset(
			textures + previousBufferSize,
			0,
			newBufferSize - previousBufferSize);
	}

	if (numTextures + amount == 1)
	{
		printf(
			"Increased textures capacity to %d to hold 1 new texture\n", texturesCapacity);
	}
	else
	{
		printf(
			"Increased textures capacity to %d to hold %d new textures\n", texturesCapacity,
			amount);
	}
}

Texture* getTexture(const char *name)
{
	if (name && strlen(name) > 0)
	{
		for (uint32 i = 0; i < numTextures; i++)
		{
			if (!strcmp(textures[i].name, name))
			{
				return &textures[i];
			}
		}
	}

	return NULL;
}

uint32 getTextureIndex(const char *name)
{
	if (name && strlen(name) > 0)
	{
		for (uint32 i = 0; i < numTextures; i++)
		{
			if (!strcmp(textures[i].name, name))
			{
				return i;
			}
		}
	}

	return 0;
}

int32 freeTexture(const char *name)
{
	printf("Freeing texture (%s)...\n", name);

	Texture *texture = getTexture(name);

	if (!texture)
	{
		printf("Could not find texture (%s)\n", name);
		return -1;
	}

	uint32 index = getTextureIndex(name);

	if (--(texture->refCount) == 0)
	{
		free(texture->name);
		glDeleteTextures(1, &texture->id);

		numTextures--;
		Texture *resizedTextures = calloc(texturesCapacity, sizeof(Texture));
		memcpy(resizedTextures, textures, index * sizeof(Texture));

		if (index < numTextures)
		{
			memcpy(
				&resizedTextures[index],
				&textures[index + 1],
				(numTextures - index) * sizeof(Texture));
		}

		free(textures);
		textures = resizedTextures;

		printf("Successfully freed texture (%s)\n", name);
		printf("Texture Count: %d\n", numTextures);
	}
	else
	{
		printf(
			"Successfully reduced texture (%s) reference count to %d\n",
			name,
			texture->refCount);
	}

	return 0;
}
