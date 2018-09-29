#include "asset_management/texture.h"

#include "core/config.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include "ECS/scene.h"

#include "renderer/renderer_utilities.h"

#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_LINEAR
#define STB_IMAGE_IMPLEMENTATION

#include <stb/stb_image.h>

#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_TEXTURE_FILE_FORMATS 6
internal const char* textureFileFormats[NUM_TEXTURE_FILE_FORMATS] = {
	"tga", "png", "jpg", "bmp", "gif", "psd"
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
	if (!hashMapGetData(textures, &nameID))
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
			texture.lifetime = config.assetsConfig.minTextureLifetime;

			error = loadTextureData(
				ASSET_LOG_TYPE_TEXTURE,
				"texture",
				name,
				filename,
				0,
				&texture.data);

			if (error != - 1)
			{
				pthread_mutex_lock(&uploadTexturesMutex);
				hashMapInsert(uploadTexturesQueue, &nameID, &texture);
				pthread_mutex_unlock(&uploadTexturesMutex);

				ASSET_LOG(
					TEXTURE,
					name,
					"Successfully loaded texture (%s)\n",
					textureName);
			}

			ASSET_LOG_COMMIT(TEXTURE, name);

			pthread_mutex_lock(&loadingTexturesMutex);
			hashMapDelete(loadingTextures, &nameID);
			pthread_mutex_unlock(&loadingTexturesMutex);
		}
	}
	else
	{
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
	int32 numComponents,
	TextureData *data)
{
	data->data = stbi_load(
		filename,
		&data->width,
		&data->height,
		&data->numComponents,
		0);

	if (!data->data)
	{
		if (name)
		{
			ASSET_LOG_FULL_TYPE(
				type,
				name,
				"Failed to load %s\n",
				typeName);
		}
		else
		{
			LOG("Failed to load %s\n", typeName);
		}

		return -1;
	}

	return 0;
}

int32 uploadTextureToGPU(
	const char *name,
	const char *type,
	GLuint *id,
	TextureData *data,
	bool textureFiltering,
	bool transparent)
{
	LOG("Transferring %s (%s) onto GPU...\n", type, name);

	glGenTextures(1, id);
	glBindTexture(GL_TEXTURE_2D, *id);

	GLint format = GL_RGBA;
	switch (data->numComponents)
	{
		case 1:
			format = GL_R;
			break;
		case 2:
			format = GL_RG;
			break;
		case 3:
			format = GL_RGB;
			break;
		default:
			break;
	}

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		format,
		data->width,
		data->height,
		0,
		format,
		GL_UNSIGNED_BYTE,
		data->data);

	free(data->data);

	int32 error = logGLError(false, "Failed to transfer %s onto GPU", type);

	if (error != -1)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER,
			textureFiltering ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER,
			textureFiltering ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);

		if (transparent)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		LOG("Successfully transferred %s (%s) onto GPU\n", type, name);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	return error;
}

Texture getTexture(const char *name)
{
	Texture texture = {};
	if (strlen(name) > 0)
	{
		UUID textureName = idFromName(name);

		pthread_mutex_lock(&texturesMutex);

		Texture *textureResource = hashMapGetData(textures, &textureName);
		if (textureResource)
		{
			textureResource->lifetime = config.assetsConfig.minTextureLifetime;
			texture = *textureResource;
		}

		pthread_mutex_unlock(&texturesMutex);
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

void freeTextureData(Texture *texture)
{
	LOG("Freeing texture (%s)...\n", texture->name.string);

	glDeleteTextures(1, &texture->id);

	LOG("Successfully freed texture (%s)\n", texture->name.string);
}