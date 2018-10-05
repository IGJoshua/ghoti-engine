#include "asset_management/asset_manager.h"
#include "asset_management/cubemap.h"
#include "asset_management/texture.h"

#include "core/log.h"
#include "core/config.h"

#include "file/utilities.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include "ECS/scene.h"

#include "renderer/renderer_utilities.h"

#include <pthread.h>

internal const char* cubemapFaceNames[6] = {
	"right",
	"left",
	"top",
	"bottom",
	"back",
	"front"
};

typedef struct cubemap_thread_args_t
{
	char *name;
	bool swapFrontAndBack;
} CubemapThreadArgs;

extern Config config;

EXTERN_ASSET_VARIABLES(cubemaps, Cubemaps);
EXTERN_ASSET_MANAGER_VARIABLES;

INTERNAL_ASSET_THREAD_VARIABLES(Cubemap);

internal void getFullCubemapFilenames(const char *name, char **filenames);

void loadCubemap(const char *name, bool swapFrontAndBack)
{
	CubemapThreadArgs *arg = malloc(sizeof(CubemapThreadArgs));

	arg->name = calloc(1, strlen(name) + 1);
	strcpy(arg->name, name);

	arg->swapFrontAndBack = swapFrontAndBack;

	bool skip = false;

	UUID cubemapName = idFromName(name);

	pthread_mutex_lock(&cubemapsMutex);
	if (!hashMapGetData(cubemaps, &cubemapName))
	{
		pthread_mutex_unlock(&cubemapsMutex);
		pthread_mutex_lock(&loadingCubemapsMutex);

		if (hashMapGetData(loadingCubemaps, &cubemapName))
		{
			skip = true;
		}

		pthread_mutex_unlock(&loadingCubemapsMutex);

		if (!skip)
		{
			pthread_mutex_lock(&uploadCubemapsMutex);

			if (hashMapGetData(uploadCubemapsQueue, &cubemapName))
			{
				skip = true;
			}

			pthread_mutex_unlock(&uploadCubemapsMutex);
		}

		if (!skip)
		{
			START_ACQUISITION_THREAD(
				cubemap,
				Cubemap,
				Cubemaps,
				arg,
				cubemapName);
			return;
		}
	}
	else
	{
		pthread_mutex_unlock(&cubemapsMutex);
	}

	free(arg->name);
	free(arg);
}

ACQUISITION_THREAD(Cubemap);

void* loadCubemapThread(void *arg)
{
	int32 error = 0;

	CubemapThreadArgs *threadArgs = arg;
	char *name = threadArgs->name;
	bool swapFrontAndBack = threadArgs->swapFrontAndBack;

	UUID cubemapName = idFromName(name);

	char *fullFilenames[6];
	getFullCubemapFilenames(name, fullFilenames);

	ASSET_LOG(CUBEMAP, name, "Loading cubemap (%s)...\n", name);

	Cubemap cubemap = {};

	cubemap.name = idFromName(name);
	cubemap.lifetime = config.assetsConfig.minCubemapLifetime;

	for (uint8 i = 0; i < 6; i++)
	{
		char *faceName = concatenateStrings(
			cubemapFaceNames[i],
			" ",
			"face");

		char *fullFilename = fullFilenames[i];
		if (!fullFilename)
		{
			ASSET_LOG(
				CUBEMAP,
				name,
				"Failed to load %s: Texture not found\n",
				faceName);
			free(faceName);

			continue;
		}

		const char *fullName = strrchr(fullFilename, '/');
		if (!fullName)
		{
			fullName = fullFilename;
		}
		else
		{
			fullName += 1;
		}

		ASSET_LOG(
			CUBEMAP,
			name,
			"Loading %s (%s)...\n",
			faceName,
			fullName);

		error = loadTextureData(
			ASSET_LOG_TYPE_CUBEMAP,
			faceName,
			name,
			fullFilename,
			3,
			&cubemap.data[i]);

		if (error != -1)
		{
			ASSET_LOG(
				CUBEMAP,
				name,
				"Successfully loaded %s (%s)\n",
				faceName,
				fullName);
			free(faceName);
		}
		else
		{
			free(faceName);
			break;
		}
	}

	for (uint8 i = 0; i < 6; i++)
	{
		free(fullFilenames[i]);
	}

	if (error != - 1)
	{
		if (swapFrontAndBack)
		{
			TextureData data = cubemap.data[4];
			cubemap.data[4] = cubemap.data[5];
			cubemap.data[5] = data;
		}

		pthread_mutex_lock(&uploadCubemapsMutex);
		hashMapInsert(uploadCubemapsQueue, &cubemapName, &cubemap);
		pthread_mutex_unlock(&uploadCubemapsMutex);

		ASSET_LOG(
			CUBEMAP,
			name,
			"Successfully loaded cubemap (%s)\n",
			name);
	}

	ASSET_LOG_COMMIT(CUBEMAP, name);

	pthread_mutex_lock(&loadingCubemapsMutex);
	hashMapDelete(loadingCubemaps, &cubemapName);
	pthread_mutex_unlock(&loadingCubemapsMutex);

	free(arg);
	free(name);

	EXIT_LOADING_THREAD;
}

int32 uploadCubemapToGPU(Cubemap *cubemap)
{
	int32 error = 0;

	LOG("Transferring cubemap (%s) onto GPU...\n", cubemap->name.string);

	glGenTextures(1, &cubemap->id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->id);

	for (uint8 i = 0; i < 6; i++)
	{
		TextureData *data = &cubemap->data[i];

		if (!data)
		{
			continue;
		}

		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB,
			data->width,
			data->height,
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			data->data);

		error = logGLError(
			false,
			"Failed to transfer cubemap's %s face onto GPU",
			cubemapFaceNames[i]);

		if (error == -1)
		{
			break;
		}
	}

	for (uint8 i = 0; i < 6; i++)
	{
		free(cubemap->data[i].data);
	}

	if (error != -1)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(
			GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_WRAP_S,
			GL_CLAMP_TO_EDGE);
		glTexParameteri(
			GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_WRAP_T,
			GL_CLAMP_TO_EDGE);
		glTexParameteri(
			GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_WRAP_R,
			GL_CLAMP_TO_EDGE);

		LOG("Successfully transferred cubemap (%s) onto GPU\n",
			cubemap->name.string);
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return error;
}

GET_ASSET_FUNCTION(
	cubemap,
	cubemaps,
	Cubemap,
	getCubemap(const char *name),
	idFromName(name));

void freeCubemapData(Cubemap *cubemap)
{
	LOG("Freeing cubemap (%s)...\n", cubemap->name.string);

	glDeleteTextures(1, &cubemap->id);

	LOG("Successfully freed cubemap (%s)\n", cubemap->name.string);
}

void getFullCubemapFilenames(const char *name, char **filenames)
{
	char *folder = getFullFilePath(name, NULL, "resources/cubemaps");
	char *filenamePrefix = getFullFilePath(name, NULL, folder);

	for (uint8 i = 0; i < 6; i++)
	{
		char *filename = concatenateStrings(
			filenamePrefix,
			"_",
			cubemapFaceNames[i]);

		filenames[i] = getFullTextureFilename(filename);

		free(filename);
	}

	free(folder);
	free(filenamePrefix);
}