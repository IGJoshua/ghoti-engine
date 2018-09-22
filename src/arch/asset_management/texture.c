#include "asset_management/asset_manager_types.h"
#include "asset_management/texture.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include "ECS/scene.h"

#include "renderer/renderer_utilities.h"

#include <IL/il.h>
#include <IL/ilu.h>

#include <string.h>
#include <unistd.h>
#include <pthread.h>

extern HashMap textures;
extern pthread_mutex_t texturesMutex;

#define NUM_TEXTURE_FILE_FORMATS 7
internal const char* textureFileFormats[NUM_TEXTURE_FILE_FORMATS] = {
	"tga", "png", "jpg", "dds", "bmp", "gif", "hdr"
};

int32 loadTexture(const char *filename, const char *name)
{
	int32 error = 0;

	UUID nameID = idFromName(name);
	pthread_mutex_lock(&texturesMutex);
	Texture *textureResource = hashMapGetData(textures, &nameID);
	pthread_mutex_unlock(&texturesMutex);

	if (!textureResource)
	{
		const char *textureName = strrchr(filename, '/');
		if (!textureName)
		{
			textureName = filename;
		}
		else
		{
			textureName += 1;
		}

		ASSET_LOG("Loading texture (%s)...\n", textureName);

		Texture texture = {};

		texture.name = idFromName(name);
		texture.refCount = 1;

		error = loadTextureData(
			filename,
			TEXTURE_FORMAT_RGBA8,
			&texture.devilID);

		ilBindImage(0);

		if (error != - 1)
		{
			pthread_mutex_lock(&texturesMutex);

			hashMapInsert(textures, &texture.name, &texture);

			ASSET_LOG("Successfully loaded texture (%s)\n", textureName);
			ASSET_LOG("Texture Count: %d\n", textures->count);

			pthread_mutex_unlock(&texturesMutex);
		}
	}
	else
	{
		textureResource->refCount++;
	}

	return error;
}

int32 loadTextureData(
	const char *filename,
	TextureFormat format,
	ILuint *devilID)
{
	ilGenImages(1, devilID);
	ilBindImage(*devilID);

	ilLoadImage(filename);

	ILenum ilError = ilGetError();
	if (ilError != IL_NO_ERROR)
	{
		ASSET_LOG("Failed to load texture: %s\n", iluErrorString(ilError));
		return -1;
	}

	ILenum ilColorFormat = IL_RGBA;
	ILenum ilByteFormat = IL_UNSIGNED_BYTE;

	switch (format)
	{
		case TEXTURE_FORMAT_R8:
			ilColorFormat = IL_LUMINANCE;
			ilByteFormat = IL_UNSIGNED_BYTE;
			break;
		case TEXTURE_FORMAT_RGBA8:
		default:
			ilColorFormat = IL_RGBA;
			ilByteFormat = IL_UNSIGNED_BYTE;
			break;
	}

	ilConvertImage(ilColorFormat, ilByteFormat);

	return 0;
}

int32 uploadTextureToGPU(Texture *texture)
{
	LOG("Transferring texture data onto GPU...\n");

	ilBindImage(texture->devilID);

	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);

	GLsizei textureWidth = ilGetInteger(IL_IMAGE_WIDTH);
	GLsizei textureHeight = ilGetInteger(IL_IMAGE_HEIGHT);

	glTexStorage2D(
		GL_TEXTURE_2D,
		1,
		GL_RGBA8,
		textureWidth,
		textureHeight);

	const GLvoid *textureData = ilGetData();
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

	int32 error = logGLError(false, "Failed to transfer texture onto GPU");

	if (error != -1)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER,
			GL_LINEAR);
		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);

		texture->uploaded = true;

		LOG("Successfully transferred texture data onto GPU\n");
	}

	ilDeleteImages(1, &texture->devilID);

	glBindTexture(GL_TEXTURE_2D, 0);
	ilBindImage(0);

	return error;
}

Texture* getTexture(const char *name)
{
	Texture *texture = NULL;
	if (strlen(name) > 0)
	{
		UUID nameID = idFromName(name);

		pthread_mutex_lock(&texturesMutex);
		texture = hashMapGetData(textures, &nameID);
		pthread_mutex_unlock(&texturesMutex);
	}

	if (texture && texture->refCount == 0)
	{
		texture = NULL;
	}

	return texture;
}

char* getFullTextureFilename(const char *filename)
{
	char *fullFilename = malloc(strlen(filename) + 5);
	for (uint32 i = 0; i < NUM_TEXTURE_FILE_FORMATS; i++)
	{
		sprintf(fullFilename, "%s.%s", filename, textureFileFormats[i]);
		if (access(fullFilename, F_OK) != -1)
		{
			return fullFilename;
		}
	}

	free(fullFilename);

	return NULL;
}

void freeTexture(UUID name)
{
	Texture *texture = (Texture*)hashMapGetData(textures, &name);
	if (texture)
	{
		texture->refCount--;
	}
}

void freeTextureData(Texture *texture)
{
	LOG("Freeing texture data (%s)...\n", texture->name.string);

	if (texture->uploaded)
	{
		glDeleteTextures(1, &texture->id);
	}

	LOG("Successfully freed texture data (%s)\n", texture->name.string);
}