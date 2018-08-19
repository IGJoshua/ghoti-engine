#include "asset_management/asset_manager_types.h"
#include "asset_management/texture.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include "ECS/scene.h"

#include <IL/il.h>
#include <IL/ilu.h>

#include <string.h>
#include <unistd.h>

extern HashMap textures;

#define NUM_TEXTURE_FILE_FORMATS 7
const char* textureFileFormats[NUM_TEXTURE_FILE_FORMATS] = {
	"tga", "png", "jpg", "dds", "bmp", "gif", "hdr"
};

int32 loadTexture(const char *filename, const char *name)
{
	int32 error = 0;

	Texture *textureResource = getTexture(name);
	if (!textureResource)
	{
		Texture texture = {};

		char *textureName = strrchr(filename, '/') + 1;

		LOG("Loading texture (%s)...\n", textureName);

		texture.name = idFromName(name);
		texture.refCount = 1;

		ILuint devilID;
		error = loadTextureData(filename, TEXTURE_FORMAT_RGBA8, &devilID);

		if (error != -1)
		{
			glGenTextures(1, &texture.id);
			glBindTexture(GL_TEXTURE_2D, texture.id);

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

			GLenum glError = glGetError();
			if (glError != GL_NO_ERROR)
			{
				LOG("Error while transferring texture onto GPU: %s\n",
					gluErrorString(glError));
				error = -1;
			}

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

				glBindTexture(GL_TEXTURE_2D, 0);
				ilDeleteImages(1, &devilID);
			}
		}

		if (error != - 1)
		{
			hashMapInsert(&textures, &texture.name, &texture);

			LOG("Successfully loaded texture (%s)\n", textureName);
			LOG("Texture Count: %d\n", textures->count);
		}
		else
		{
			LOG("Failed to load texture (%s)\n", textureName);
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
		LOG("Error while loading texture: %s\n", iluErrorString(ilError));
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

Texture* getTexture(const char *name)
{
	Texture *texture = NULL;
	if (strlen(name) > 0)
	{
		UUID nameID = idFromName(name);
		texture = hashMapGetData(&textures, &nameID);
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
	Texture *texture = (Texture*)hashMapGetData(&textures, &name);
	if (texture)
	{
		if (--texture->refCount == 0)
		{
			LOG("Freeing texture (%s)...\n", name.string);

			glDeleteTextures(1, &texture->id);
			hashMapDelete(&textures, &name);

			LOG("Successfully freed texture (%s)\n", name.string);
			LOG("Texture Count: %d\n", textures->count);
		}
	}
}