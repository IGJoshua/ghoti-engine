#include "asset_management/asset_manager_types.h"
#include "asset_management/texture.h"

#include "core/config.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include "ECS/scene.h"

#include "renderer/renderer_utilities.h"

#include <IL/il.h>
#include <IL/ilu.h>

#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_TEXTURE_FILE_FORMATS 7
internal const char* textureFileFormats[NUM_TEXTURE_FILE_FORMATS] = {
	"tga", "png", "jpg", "dds", "bmp", "gif", "hdr"
};

typedef struct texture_thread_args_t
{
	char *filename;
	char *name;
} TextureThreadArgs;

extern Config config;

extern HashMap textures;
extern pthread_mutex_t texturesMutex;

extern HashMap loadingTextures;
extern pthread_mutex_t loadingTexturesMutex;

extern HashMap uploadTexturesQueue;
extern pthread_mutex_t uploadTexturesMutex;

extern uint32 assetThreadCount;
extern pthread_mutex_t assetThreadsMutex;
extern pthread_cond_t assetThreadsCondition;

extern pthread_mutex_t devilMutex;

internal void* acquireTextureThread(void *arg);
internal void* loadTextureThread(void *arg);

void loadTexture(const char *filename, const char *name)
{
	TextureThreadArgs *arg = malloc(sizeof(TextureThreadArgs));

	arg->filename = calloc(1, strlen(filename) + 1);
	strcpy(arg->filename, filename);

	arg->name = calloc(1, strlen(name) + 1);
	strcpy(arg->name, name);

	pthread_t acquisitionThread;
	pthread_create(&acquisitionThread, NULL, &acquireTextureThread, (void*)arg);
	pthread_detach(acquisitionThread);
}

void* acquireTextureThread(void *arg)
{
	pthread_mutex_lock(&assetThreadsMutex);

	while (assetThreadCount == config.assetsConfig.maxThreadCount)
	{
		pthread_cond_wait(&assetThreadsCondition, &assetThreadsMutex);
	}

	assetThreadCount++;

	pthread_mutex_unlock(&assetThreadsMutex);
	pthread_cond_broadcast(&assetThreadsCondition);

	pthread_t loadingThread;
	pthread_create(&loadingThread, NULL, &loadTextureThread, arg);
	pthread_detach(loadingThread);

	EXIT_THREAD(NULL);
}

void* loadTextureThread(void *arg)
{
	int32 error = 0;

	TextureThreadArgs *threadArgs = arg;
	char *filename = threadArgs->filename;
	char *name = threadArgs->name;

	UUID nameID = idFromName(name);

	pthread_mutex_lock(&texturesMutex);
	Texture *textureResource = hashMapGetData(textures, &nameID);

	if (!textureResource)
	{
		pthread_mutex_unlock(&texturesMutex);
		pthread_mutex_lock(&loadingTexturesMutex);

		if (hashMapGetData(loadingTextures, &nameID))
		{
			error = 1;
		}

		pthread_mutex_unlock(&loadingTexturesMutex);

		if (error != 1)
		{
			pthread_mutex_lock(&uploadTexturesMutex);

			if (hashMapGetData(uploadTexturesQueue, &nameID))
			{
				error = 1;
			}

			pthread_mutex_unlock(&uploadTexturesMutex);
		}

		if (error != 1)
		{
			bool loading = true;
			pthread_mutex_lock(&loadingTexturesMutex);
			hashMapInsert(loadingTextures, &nameID, &loading);
			pthread_mutex_unlock(&loadingTexturesMutex);

			const char *textureName = strrchr(filename, '/');
			if (!textureName)
			{
				textureName = filename;
			}
			else
			{
				textureName += 1;
			}

			ASSET_LOG(TEXTURE, name, "Loading texture (%s)...\n", textureName);

			Texture texture = {};

			texture.name = nameID;
			texture.refCount = 1;

			error = loadTextureData(
				ASSET_LOG_TYPE_TEXTURE,
				"texture",
				name,
				filename,
				TEXTURE_FORMAT_RGBA8,
				&texture.devilID);

			ilBindImage(0);

			if (error != - 1)
			{
				pthread_mutex_lock(&uploadTexturesMutex);
				hashMapInsert(uploadTexturesQueue, &nameID, &texture);
				pthread_mutex_unlock(&uploadTexturesMutex);

				pthread_mutex_lock(&loadingTexturesMutex);
				hashMapDelete(loadingTextures, &nameID);
				pthread_mutex_unlock(&loadingTexturesMutex);

				ASSET_LOG(
					TEXTURE,
					name,
					"Successfully loaded texture (%s)\n",
					textureName);
			}

			ASSET_LOG_COMMIT(TEXTURE, name);
		}
	}
	else
	{
		textureResource->refCount++;
		pthread_mutex_unlock(&texturesMutex);
	}

	free(arg);
	free(filename);
	free(name);

	pthread_mutex_lock(&assetThreadsMutex);
	assetThreadCount--;
	pthread_mutex_unlock(&assetThreadsMutex);
	pthread_cond_broadcast(&assetThreadsCondition);

	EXIT_THREAD(NULL);
}

int32 loadTextureData(
	AssetLogType type,
	const char *typeName,
	const char *name,
	const char *filename,
	TextureFormat format,
	ILuint *devilID)
{
	pthread_mutex_lock(&devilMutex);

	ilGenImages(1, devilID);
	ilBindImage(*devilID);

	ilLoadImage(filename);

	ILenum ilError = ilGetError();
	if (ilError != IL_NO_ERROR)
	{
		if (name)
		{
			ASSET_LOG_FULL_TYPE(
				type,
				name,
				"Failed to load %s: %s\n",
				typeName,
				iluErrorString(ilError));
		}
		else
		{
			LOG("Failed to load %s: %s\n",
				typeName,
				iluErrorString(ilError));
		}

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

	pthread_mutex_unlock(&devilMutex);

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

		LOG("Successfully transferred texture data onto GPU\n");
	}

	ilDeleteImages(1, &texture->devilID);

	glBindTexture(GL_TEXTURE_2D, 0);
	ilBindImage(0);

	return error;
}

Texture getTexture(const char *name)
{
	Texture texture = {};
	if (strlen(name) > 0)
	{
		UUID nameID = idFromName(name);

		pthread_mutex_lock(&texturesMutex);
		Texture *textureResource = hashMapGetData(textures, &nameID);
		pthread_mutex_unlock(&texturesMutex);

		if (textureResource)
		{
			texture = *textureResource;
		}
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
	pthread_mutex_lock(&texturesMutex);
	Texture *texture = (Texture*)hashMapGetData(textures, &name);
	if (texture)
	{
		texture->refCount--;
	}

	pthread_mutex_unlock(&texturesMutex);
}

void freeTextureData(Texture *texture)
{
	LOG("Freeing texture data (%s)...\n", texture->name.string);

	glDeleteTextures(1, &texture->id);

	LOG("Successfully freed texture data (%s)\n", texture->name.string);
}