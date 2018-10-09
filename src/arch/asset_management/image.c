#include "asset_management/asset_manager.h"
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

EXTERN_ASSET_VARIABLES(images, Images);
EXTERN_ASSET_MANAGER_VARIABLES;

INTERNAL_ASSET_THREAD_VARIABLES(Image);

internal char* getFullImageFilename(const char *name);

void loadImage(const char *name, bool textureFiltering)
{
	ImageThreadArgs *arg = malloc(sizeof(ImageThreadArgs));

	arg->name = calloc(1, strlen(name) + 1);
	strcpy(arg->name, name);

	arg->textureFiltering = textureFiltering;

	bool skip = false;

	UUID nameID = idFromName(name);

	pthread_mutex_lock(&imagesMutex);
	if (!hashMapGetData(images, &nameID))
	{
		pthread_mutex_unlock(&imagesMutex);
		pthread_mutex_lock(&loadingImagesMutex);

		if (hashMapGetData(loadingImages, &nameID))
		{
			skip = true;
		}

		pthread_mutex_unlock(&loadingImagesMutex);

		if (!skip)
		{
			pthread_mutex_lock(&uploadImagesMutex);

			if (hashMapGetData(uploadImagesQueue, &nameID))
			{
				skip = true;
			}

			pthread_mutex_unlock(&uploadImagesMutex);
		}

		if (!skip)
		{
			START_ACQUISITION_THREAD(image, Image, Images, arg, nameID);
			return;
		}
	}
	else
	{
		pthread_mutex_unlock(&imagesMutex);
	}

	free(arg->name);
	free(arg);
}

ACQUISITION_THREAD(Image);

void* loadImageThread(void *arg)
{
	int32 error = 0;

	ImageThreadArgs *threadArgs = arg;
	char *name = threadArgs->name;
	bool textureFiltering = threadArgs->textureFiltering;

	UUID nameID = idFromName(name);

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
			4,
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

	free(arg);
	free(name);

	EXIT_LOADING_THREAD;
}

GET_ASSET_FUNCTION(
	image,
	images,
	Image,
	getImage(const char *name),
	idFromName(name));

void freeImageData(Image *image)
{
	LOG("Freeing image (%s)...\n", image->name.string);

	free(image->data.data);
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