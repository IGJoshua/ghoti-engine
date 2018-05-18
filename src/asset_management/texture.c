#include "asset_management/texture.h"

#include <assimp/cimport.h>

#include <IL/il.h>

#include <malloc.h>

extern Texture *textures;
extern uint32 numTextures;

int32 loadTexture(const struct aiString *name, TextureType type)
{
	Texture *texture = &textures[numTextures++];

	texture->name = malloc(name->length + 1);
	strcpy(texture->name, name->data);

	texture->type = type;

	ILuint devilID;
	ilGenImages(1, &devilID);
	ilBindImage(devilID);
	
	char filename[1024];
	sprintf(filename, "resources/textures/%s", texture->name);
	ilLoadImage(filename);

	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

	GLuint textureID;
	glGenTextures(1, &textureID);

	texture->id = textureID;

	glBindTexture(GL_TEXTURE_2D, textureID);

	GLsizei textureWidth = ilGetInteger(IL_IMAGE_WIDTH);
	GLsizei textureHeight = ilGetInteger(IL_IMAGE_HEIGHT);
	const GLvoid *textureData = ilGetData();
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA8,
		textureWidth,
		textureHeight,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		textureData
	);

	// TODO Add mipmapping
	// glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);

	ilDeleteImages(1, &devilID);

	texture->refCount++;

	return 0;
}

Texture* getTexture(const char *name)
{
	for (uint32 i = 0; i < numTextures; i++)
	{
		if (strcmp(textures[i].name, name) == 0)
		{
			return &textures[i];
		}
	}

	return NULL;
}

uint32 getTextureIndex(const char *name)
{
	for (uint32 i = 0; i < numTextures; i++)
	{
		if (strcmp(textures[i].name, name) == 0)
		{
			return i;
		}
	}

	return -1;
}

int32 freeTexture(const char *name)
{
	Texture *texture = getTexture(name);
	uint32 index = getTextureIndex(name);
	
	if (texture)
	{
		if (--(texture->refCount) == 0)
		{
			free(texture->name);
			glDeleteTextures(1, &texture->id);

			Texture *resizedTextures = malloc(--numTextures * sizeof(Texture));
			memcpy(resizedTextures, textures, index * sizeof(Texture));

			if (index < numTextures)
			{
				memcpy(
					resizedTextures + index * sizeof(Texture),
					texture + sizeof(Texture),
					(numTextures - index) * sizeof(Texture)
				);
			}

			free(textures);
			textures = resizedTextures;
		}
	}

	return 0;
}
