#include "asset_management/asset_manager.h"
#include "asset_management/cubemap.h"
#include "asset_management/cubemap_importer.h"
#include "asset_management/texture.h"

#include "core/log.h"
#include "core/config.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include "ECS/scene.h"

#include "file/utilities.h"

#include "renderer/renderer_utilities.h"

#include <pthread.h>
#include <unistd.h>

extern Config config;

EXTERN_ASSET_VARIABLES(cubemaps, Cubemaps);
EXTERN_ASSET_MANAGER_VARIABLES;

INTERNAL_ASSET_THREAD_VARIABLES(Cubemap);

internal char* getFullCubemapFilename(const char *name);

void loadCubemap(const char *name)
{
	char *cubemapName = calloc(1, strlen(name) + 1);
	strcpy(cubemapName, name);

	bool skip = false;

	UUID nameID = idFromName(name);

	pthread_mutex_lock(&cubemapsMutex);
	if (!hashMapGetData(cubemaps, &nameID))
	{
		pthread_mutex_unlock(&cubemapsMutex);
		pthread_mutex_lock(&loadingCubemapsMutex);

		if (hashMapGetData(loadingCubemaps, &nameID))
		{
			skip = true;
		}

		pthread_mutex_unlock(&loadingCubemapsMutex);

		if (!skip)
		{
			pthread_mutex_lock(&uploadCubemapsMutex);

			if (hashMapGetData(uploadCubemapsQueue, &nameID))
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
				cubemapName,
				nameID);
			return;
		}
	}
	else
	{
		pthread_mutex_unlock(&cubemapsMutex);
	}

	free(cubemapName);
}

ACQUISITION_THREAD(Cubemap);

void* loadCubemapThread(void *arg)
{
	int32 error = 0;

	char *name = arg;

	UUID nameID = idFromName(name);

	char *fullFilename = getFullCubemapFilename(name);
	if (!fullFilename)
	{
		error = -1;
	}
	else
	{
		const char *cubemapName = strrchr(fullFilename, '/');
		if (!cubemapName)
		{
			cubemapName = fullFilename;
		}
		else
		{
			cubemapName += 1;
		}

		ASSET_LOG(CUBEMAP, name, "Loading cubemap (%s)...\n", cubemapName);

		Cubemap cubemap = {};

		cubemap.name = idFromName(name);
		cubemap.lifetime = config.assetsConfig.minCubemapLifetime;

		error = loadHDRTextureData(
			ASSET_LOG_TYPE_CUBEMAP,
			"cubemap",
			name,
			fullFilename,
			3,
			true,
			&cubemap.data);

		if (error != - 1)
		{
			pthread_mutex_lock(&uploadCubemapsMutex);
			hashMapInsert(uploadCubemapsQueue, &nameID, &cubemap);
			pthread_mutex_unlock(&uploadCubemapsMutex);

			ASSET_LOG(
				CUBEMAP,
				name,
				"Successfully loaded cubemap (%s)\n",
				cubemapName);
		}

		ASSET_LOG_COMMIT(CUBEMAP, name);

		pthread_mutex_lock(&loadingCubemapsMutex);
		hashMapDelete(loadingCubemaps, &nameID);
		pthread_mutex_unlock(&loadingCubemapsMutex);
	}

	free(fullFilename);

	free(name);

	EXIT_LOADING_THREAD;
}

int32 uploadCubemapToGPU(Cubemap *cubemap)
{
	int32 error = 0;

	LOG("Transferring cubemap (%s) onto GPU...\n", cubemap->name.string);

	glGenTextures(1, &cubemap->equirectangularID);
	glBindTexture(GL_TEXTURE_2D, cubemap->equirectangularID);

	HDRTextureData *data = &cubemap->data;

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB16F,
		data->width,
		data->height,
		0,
		GL_RGB,
		GL_FLOAT,
		data->data);

	free(data->data);
	data->data = NULL;

	error = logGLError(false, "Failed to transfer cubemap onto GPU");

	if (error != -1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		importCubemap(cubemap);

		LOG("Successfully transferred cubemap (%s) onto GPU\n",
			cubemap->name.string);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

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

	free(cubemap->data.data);

	glDeleteTextures(1, &cubemap->equirectangularID);
	glDeleteTextures(1, &cubemap->cubemapID);
	glDeleteTextures(1, &cubemap->irradianceID);
	glDeleteTextures(1, &cubemap->prefilterID);

	LOG("Successfully freed cubemap (%s)\n", cubemap->name.string);
}

char* getFullCubemapFilename(const char *name)
{
	char *filename = getFullFilePath(name, "hdr", "resources/cubemaps");
	if (access(filename, F_OK) != -1)
	{
		return filename;
	}

	free(filename);

	return NULL;
}