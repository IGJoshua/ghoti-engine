#include "asset_management/asset_manager.h"
#include "asset_management/cubemap.h"
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

const char *cubemapFaceNames[6] = { "+x", "-x", "+y", "-y", "+z", "-z" };

extern Config config;

EXTERN_ASSET_VARIABLES(cubemaps, Cubemaps);
EXTERN_ASSET_MANAGER_VARIABLES;

INTERNAL_ASSET_THREAD_VARIABLES(Cubemap);

internal char* getCubemapFolder(const char *name);

internal int32 loadEnvironmentMap(const char *cubemapFolder, Cubemap *cubemap);
internal int32 loadIrradianceMap(const char *cubemapFolder, Cubemap *cubemap);
internal int32 loadPrefilterMap(const char *cubemapFolder, Cubemap *cubemap);

internal int32 uploadEnvironmentMap(Cubemap *cubemap);
internal int32 uploadIrradianceMap(Cubemap *cubemap);
internal int32 uploadPrefilterMap(Cubemap *cubemap);

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

	char *cubemapFolder = getCubemapFolder(name);
	if (!cubemapFolder)
	{
		error = -1;
	}
	else
	{
		ASSET_LOG(CUBEMAP, name, "Loading cubemap (%s)...\n", name);

		Cubemap cubemap = {};

		cubemap.name = idFromName(name);
		cubemap.lifetime = config.assetsConfig.minCubemapLifetime;

		error = loadEnvironmentMap(cubemapFolder, &cubemap);

		if (error != -1)
		{
			error = loadIrradianceMap(cubemapFolder, &cubemap);

			if (error != -1)
			{
				error = loadPrefilterMap(cubemapFolder, &cubemap);
			}
		}

		if (error != - 1)
		{
			pthread_mutex_lock(&uploadCubemapsMutex);
			hashMapInsert(uploadCubemapsQueue, &nameID, &cubemap);
			pthread_mutex_unlock(&uploadCubemapsMutex);

			ASSET_LOG(
				CUBEMAP,
				name,
				"Successfully loaded cubemap (%s)\n",
				name);
		}
		else
		{
			for (uint8 i = 0; i < 6; i++)
			{
				free(cubemap.cubemapData[i].data);
				free(cubemap.irradianceData[i].data);
			}

			for (uint8 i = 0; i < 6; i++)
			{
				for (uint8 j = 0; j < 6; j++)
				{
					free(cubemap.prefilterData[i][j].data);
				}
			}
		}

		ASSET_LOG_COMMIT(CUBEMAP, name);

		pthread_mutex_lock(&loadingCubemapsMutex);
		hashMapDelete(loadingCubemaps, &nameID);
		pthread_mutex_unlock(&loadingCubemapsMutex);
	}

	free(cubemapFolder);
	free(name);

	EXIT_LOADING_THREAD;
}

int32 uploadCubemapToGPU(Cubemap *cubemap)
{
	int32 error = 0;

	LOG("Transferring cubemap (%s) onto GPU...\n", cubemap->name.string);

	error = uploadEnvironmentMap(cubemap);

	if (error != -1)
	{
		error = uploadIrradianceMap(cubemap);

		if (error != -1)
		{
			error = uploadPrefilterMap(cubemap);
		}
	}

	if (error != -1)
	{
		LOG("Successfully transferred cubemap (%s) onto GPU\n",
			cubemap->name.string);
	}

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

	for (uint8 i = 0; i < 6; i++)
	{
		free(cubemap->cubemapData[i].data);
		free(cubemap->irradianceData[i].data);
	}

	for (uint8 i = 0; i < 5; i++)
	{
		for (uint8 j = 0; j < 6; j++)
		{
			free(cubemap->prefilterData[i][j].data);
		}
	}

	glDeleteTextures(1, &cubemap->cubemapID);
	glDeleteTextures(1, &cubemap->irradianceID);
	glDeleteTextures(1, &cubemap->prefilterID);

	LOG("Successfully freed cubemap (%s)\n", cubemap->name.string);
}

char* getCubemapFolder(const char *name)
{
	char *folder = getFullFilePath(name, NULL, "resources/cubemaps");
	if (access(folder, F_OK) != -1)
	{
		return folder;
	}

	free(folder);

	return NULL;
}

int32 loadEnvironmentMap(const char *cubemapFolder, Cubemap *cubemap)
{
	int32 error = 0;

	char *environmentFolder = getFullFilePath(
		"environment",
		NULL,
		cubemapFolder);
	char *filePath = getFullFilePath(
		cubemap->name.string,
		NULL,
		environmentFolder);
	free(environmentFolder);

	char *fullFilePath = concatenateStrings(filePath, "_", "environment");
	free(filePath);

	for (uint8 i = 0; i < 6; i++)
	{
		char *filename = concatenateStrings(
			fullFilePath,
			cubemapFaceNames[i],
			".hdr");

		error = loadHDRTextureData(
			ASSET_LOG_TYPE_CUBEMAP,
			"cubemap",
			cubemap->name.string,
			filename,
			3,
			false,
			&cubemap->cubemapData[i]);

		free(filename);

		if (error == -1)
		{
			break;
		}
	}

	free(fullFilePath);

	return error;
}

int32 loadIrradianceMap(const char *cubemapFolder, Cubemap *cubemap)
{
	int32 error = 0;

	char *irradianceFolder = getFullFilePath(
		"irradiance",
		NULL,
		cubemapFolder);
	char *filePath = getFullFilePath(
		cubemap->name.string,
		NULL,
		irradianceFolder);
	free(irradianceFolder);

	char *fullFilePath = concatenateStrings(filePath, "_", "irradiance");
	free(filePath);

	for (uint8 i = 0; i < 6; i++)
	{
		char *filename = concatenateStrings(
			fullFilePath,
			cubemapFaceNames[i],
			".hdr");

		error = loadHDRTextureData(
			ASSET_LOG_TYPE_CUBEMAP,
			"cubemap",
			cubemap->name.string,
			filename,
			3,
			false,
			&cubemap->irradianceData[i]);

		free(filename);

		if (error == -1)
		{
			break;
		}
	}

	free(fullFilePath);

	return error;
}

int32 loadPrefilterMap(const char *cubemapFolder, Cubemap *cubemap)
{
	int32 error = 0;

	char *prefilterFolder = getFullFilePath(
		"prefilter",
		NULL,
		cubemapFolder);

	for (uint8 i = 0; i < 5; i++)
	{
		char mipLevelFolderName[12];
		sprintf(mipLevelFolderName, "mip_level_%d", i);

		char *mipLevelFolder = getFullFilePath(
			mipLevelFolderName,
			NULL,
			prefilterFolder);

		char *filePath = getFullFilePath(
			cubemap->name.string,
			NULL,
			mipLevelFolder);
		free(mipLevelFolder);

		char *fullFilePath = concatenateStrings(filePath, "_", "prefilter");
		free(filePath);

		char extension[7];
		sprintf(extension, "_%d.hdr", i);

		for (uint8 j = 0; j < 6; j++)
		{
			char *filename = concatenateStrings(
				fullFilePath,
				cubemapFaceNames[j],
				extension);

			error = loadHDRTextureData(
				ASSET_LOG_TYPE_CUBEMAP,
				"cubemap",
				cubemap->name.string,
				filename,
				3,
				false,
				&cubemap->prefilterData[i][j]);

			free(filename);

			if (error == -1)
			{
				break;
			}
		}

		free(fullFilePath);
	}

	free(prefilterFolder);

	return error;
}

int32 uploadEnvironmentMap(Cubemap *cubemap)
{
	int32 error = 0;

	glGenTextures(1, &cubemap->cubemapID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->cubemapID);

	for (uint8 i = 0; i < 6; i++)
	{
		HDRTextureData *data = &cubemap->cubemapData[i];
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB16F,
			data->width,
			data->height,
			0,
			GL_RGB,
			GL_FLOAT,
			data->data);

		error = logGLError(false, "Failed to transfer cubemap onto GPU");

		if (error == -1)
		{
			break;
		}
	}

	if (error != -1)
	{
		glTexParameteri(
			GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_MIN_FILTER,
			GL_LINEAR);
		glTexParameteri(
			GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_MAG_FILTER,
			GL_LINEAR);
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
	}

	return error;
}

int32 uploadIrradianceMap(Cubemap *cubemap)
{
	int32 error = 0;

	glGenTextures(1, &cubemap->irradianceID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->irradianceID);

	for (uint8 i = 0; i < 6; i++)
	{
		HDRTextureData *data = &cubemap->irradianceData[i];
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB16F,
			data->width,
			data->height,
			0,
			GL_RGB,
			GL_FLOAT,
			data->data);

		error = logGLError(false, "Failed to transfer cubemap onto GPU");

		if (error == -1)
		{
			break;
		}
	}

	if (error != -1)
	{
		glTexParameteri(
			GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_MIN_FILTER,
			GL_LINEAR);
		glTexParameteri(
			GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_MAG_FILTER,
			GL_LINEAR);
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
	}

	return error;
}

int32 uploadPrefilterMap(Cubemap *cubemap)
{
	int32 error = 0;

	glGenTextures(1, &cubemap->prefilterID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->prefilterID);

	for (uint8 i = 0; i < 5; i++)
	{
		for (uint8 j = 0; j < 6; j++)
		{
			HDRTextureData *data = &cubemap->prefilterData[i][j];
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + j,
				i,
				GL_RGB16F,
				data->width,
				data->height,
				0,
				GL_RGB,
				GL_FLOAT,
				data->data);

			error = logGLError(false, "Failed to transfer cubemap onto GPU");

			if (error == -1)
			{
				break;
			}
		}

		if (error == -1)
		{
			break;
		}
	}

	if (error != -1)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 4);
		glTexParameteri(
			GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
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
	}

	return error;
}