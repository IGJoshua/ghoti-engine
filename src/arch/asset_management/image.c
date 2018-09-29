#include "asset_management/image.h"
#include "asset_management/texture.h"

#include "core/log.h"
#include "core/config.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include "ECS/scene.h"

#include "file/utilities.h"

#include "renderer/renderer_utilities.h"

#include <pthread.h>

typedef struct image_thread_args_t
{
	char *name;
	bool textureFiltering;
} ImageThreadArgs;

extern Config config;

extern HashMap images;
extern pthread_mutex_t imagesMutex;

extern HashMap loadingImages;
extern pthread_mutex_t loadingImagesMutex;

extern HashMap uploadImagesQueue;
extern pthread_mutex_t uploadImagesMutex;

extern uint32 assetThreadCount;
extern pthread_mutex_t assetThreadsMutex;
extern pthread_cond_t assetThreadsCondition;

internal void* acquireImageThread(void *arg);
internal void* loadImageThread(void *arg);

internal char* getFullImageFilename(const char *name);

void loadImage(const char *name, bool textureFiltering)
{
	ImageThreadArgs *arg = malloc(sizeof(ImageThreadArgs));

	arg->name = calloc(1, strlen(name) + 1);
	strcpy(arg->name, name);

	arg->textureFiltering = textureFiltering;

	pthread_t acquisitionThread;
	pthread_create(&acquisitionThread, NULL, &acquireImageThread, (void*)arg);
	pthread_detach(acquisitionThread);
}

void* acquireImageThread(void *arg)
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
	pthread_create(&loadingThread, NULL, &loadImageThread, arg);
	pthread_detach(loadingThread);

	EXIT_THREAD(NULL);
}

void* loadImageThread(void *arg)
{
	int32 error = 0;

	ImageThreadArgs *threadArgs = arg;
	char *name = threadArgs->name;
	bool textureFiltering = threadArgs->textureFiltering;

	UUID nameID = idFromName(name);

	pthread_mutex_lock(&imagesMutex);
	if (!hashMapGetData(images, &nameID))
	{
		pthread_mutex_unlock(&imagesMutex);
		pthread_mutex_lock(&loadingImagesMutex);

		if (hashMapGetData(loadingImages, &nameID))
		{
			error = 1;
		}

		pthread_mutex_unlock(&loadingImagesMutex);

		if (error != 1)
		{
			pthread_mutex_lock(&uploadImagesMutex);

			if (hashMapGetData(uploadImagesQueue, &nameID))
			{
				error = 1;
			}

			pthread_mutex_unlock(&uploadImagesMutex);
		}

		if (error != 1)
		{
			bool loading = true;
			pthread_mutex_lock(&loadingImagesMutex);
			hashMapInsert(loadingImages, &nameID, &loading);
			pthread_mutex_unlock(&loadingImagesMutex);

			char *fullFilename = getFullImageFilename(name);
			if (!fullFilename)
			{
				error = -1;
			}
			else
			{
				const char *imageName = strrchr(fullFilename, '/');
				if (!imageName)
				{
					imageName = fullFilename;
				}
				else
				{
					imageName += 1;
				}

				ASSET_LOG(IMAGE, name, "Loading image (%s)...\n", imageName);

				Image image = {};

				image.name = idFromName(name);
				image.lifetime = config.assetsConfig.minImageLifetime;
				image.textureFiltering = textureFiltering;

				error = loadTextureData(
					ASSET_LOG_TYPE_IMAGE,
					"image",
					name,
					fullFilename,
					0,
					&image.data);

				if (error != - 1)
				{
					pthread_mutex_lock(&uploadImagesMutex);
					hashMapInsert(uploadImagesQueue, &nameID, &image);
					pthread_mutex_unlock(&uploadImagesMutex);

					ASSET_LOG(
						IMAGE,
						name,
						"Successfully loaded image (%s)\n",
						imageName);
				}

				ASSET_LOG_COMMIT(IMAGE, name);

				pthread_mutex_lock(&loadingImagesMutex);
				hashMapDelete(loadingImages, &nameID);
				pthread_mutex_unlock(&loadingImagesMutex);
			}

			free(fullFilename);
		}
	}
	else
	{
		pthread_mutex_unlock(&imagesMutex);
	}

	free(arg);
	free(name);

	pthread_mutex_lock(&assetThreadsMutex);
	assetThreadCount--;
	pthread_mutex_unlock(&assetThreadsMutex);
	pthread_cond_broadcast(&assetThreadsCondition);

	EXIT_THREAD(NULL);
}

Image getImage(const char *name)
{
	Image image = {};
	if (strlen(name) > 0)
	{
		UUID imageName = idFromName(name);

		pthread_mutex_lock(&imagesMutex);

		Image *imageResource = hashMapGetData(images, &imageName);
		if (imageResource)
		{
			imageResource->lifetime = config.assetsConfig.minImageLifetime;
			image = *imageResource;
		}

		pthread_mutex_unlock(&imagesMutex);
	}

	return image;
}

void freeImageData(Image *image)
{
	LOG("Freeing image (%s)...\n", image->name.string);

	glDeleteTextures(1, &image->id);

	LOG("Successfully freed image (%s)\n", image->name.string);
}

char* getFullImageFilename(const char *name)
{
	char *filename = getFullFilePath(name, NULL, "resources/images");
	char *fullFilename = getFullTextureFilename(filename);
	free(filename);

	return fullFilename;
}