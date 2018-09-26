#include "asset_management/asset_manager_types.h"
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

extern pthread_mutex_t devilMutex;

internal void* acquireImageThread(void *arg);
internal void* loadImageThread(void *arg);

internal char* getFullImageFilename(const char *name);

void loadImage(const char *name)
{
	char *imageName = calloc(1, strlen(name) + 1);
	strcpy(imageName, name);

	pthread_t acquisitionThread;
	pthread_create(
		&acquisitionThread,
		NULL,
		&acquireImageThread,
		(void*)imageName);
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

	char *name = arg;

	UUID nameID = idFromName(name);

	pthread_mutex_lock(&imagesMutex);
	Model *imageResource = hashMapGetData(images, &nameID);

	if (!imageResource)
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
				image.refCount = 1;

				pthread_mutex_lock(&devilMutex);

				error = loadTextureData(
					ASSET_LOG_TYPE_IMAGE,
					"image",
					name,
					fullFilename,
					TEXTURE_FORMAT_RGBA8,
					&image.devilID);
				ilBindImage(0);

				pthread_mutex_unlock(&devilMutex);

				if (error != - 1)
				{
					pthread_mutex_lock(&uploadImagesMutex);
					hashMapInsert(uploadImagesQueue, &nameID, &image);
					pthread_mutex_unlock(&uploadImagesMutex);

					pthread_mutex_lock(&loadingImagesMutex);
					hashMapDelete(loadingImages, &nameID);
					pthread_mutex_unlock(&loadingImagesMutex);

					ASSET_LOG(
						IMAGE,
						name,
						"Successfully loaded image (%s)\n",
						imageName);
				}

				ASSET_LOG_COMMIT(IMAGE, name);
			}

			free(fullFilename);
		}
	}
	else
	{
		imageResource->refCount++;
		pthread_mutex_unlock(&imagesMutex);
	}

	free(name);

	pthread_mutex_lock(&assetThreadsMutex);
	assetThreadCount--;
	pthread_mutex_unlock(&assetThreadsMutex);
	pthread_cond_broadcast(&assetThreadsCondition);

	EXIT_THREAD(NULL);
}

int32 uploadImageToGPU(Image *image)
{
	LOG("Transferring image (%s) onto GPU...\n", image->name.string);

	pthread_mutex_lock(&devilMutex);

	ilBindImage(image->devilID);

	glGenTextures(1, &image->id);
	glBindTexture(GL_TEXTURE_2D, image->id);

	image->width = ilGetInteger(IL_IMAGE_WIDTH);
	image->height = ilGetInteger(IL_IMAGE_HEIGHT);

	glTexStorage2D(
		GL_TEXTURE_2D,
		1,
		GL_RGBA8,
		image->width,
		image->height);

	const GLvoid *imageData = ilGetData();
	glTexSubImage2D(
		GL_TEXTURE_2D,
		0,
		0,
		0,
		image->width,
		image->height,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		imageData);

	ilDeleteImages(1, &image->devilID);
	ilBindImage(0);

	pthread_mutex_unlock(&devilMutex);

	int32 error = logGLError(false, "Failed to transfer image onto GPU");

	if (error != -1)
	{
		LOG("Successfully transferred image (%s) onto GPU\n",
			image->name.string);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	return error;
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
			image = *imageResource;
		}

		pthread_mutex_unlock(&imagesMutex);
	}

	return image;
}

void freeImage(const char *name)
{
	UUID imageName = idFromName(name);

	pthread_mutex_lock(&imagesMutex);

	Image *image = hashMapGetData(images, &imageName);
	if (image)
	{
		image->refCount--;
	}

	pthread_mutex_unlock(&imagesMutex);
}

void freeImageData(Image *image)
{
	LOG("Freeing image (%s)...\n", image->name.string);

	pthread_mutex_lock(&devilMutex);
	ilDeleteImages(1, &image->devilID);
	pthread_mutex_unlock(&devilMutex);

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