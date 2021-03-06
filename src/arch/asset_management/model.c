#include "asset_management/asset_manager.h"
#include "asset_management/model.h"
#include "asset_management/material.h"
#include "asset_management/mask.h"
#include "asset_management/mesh.h"
#include "asset_management/animation.h"
#include "asset_management/texture.h"

#include "core/log.h"
#include "core/config.h"

#include "file/utilities.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include "ECS/scene.h"

#include <pthread.h>

#define ASSET_BINARY_FILE_VERSION 1

extern Config config;

EXTERN_ASSET_VARIABLES(models, Models);
EXTERN_ASSET_MANAGER_VARIABLES;

INTERNAL_ASSET_THREAD_VARIABLES(Model);

internal int32 loadSubset(
	Subset *subset,
	FILE *assetFile,
	FILE *meshFile,
	const char *modelName);

void loadModel(const char *name)
{
	if (strlen(name) == 0)
	{
		return;
	}

	char *modelName = calloc(1, strlen(name) + 1);
	strcpy(modelName, name);

	bool skip = false;

	UUID nameID = idFromName(name);

	pthread_mutex_lock(&modelsMutex);
	if (!hashMapGetData(models, &nameID))
	{
		pthread_mutex_unlock(&modelsMutex);
		pthread_mutex_lock(&loadingModelsMutex);

		if (hashMapGetData(loadingModels, &nameID))
		{
			skip = true;
		}

		pthread_mutex_unlock(&loadingModelsMutex);

		if (!skip)
		{
			pthread_mutex_lock(&uploadModelsMutex);

			if (hashMapGetData(uploadModelsQueue, &nameID))
			{
				skip = 1;
			}

			pthread_mutex_unlock(&uploadModelsMutex);
		}

		if (!skip)
		{
			START_ACQUISITION_THREAD(model, Model, Models, modelName, nameID);
			return;
		}
	}
	else
	{
		pthread_mutex_unlock(&modelsMutex);
	}

	free(modelName);
}

ACQUISITION_THREAD(Model);

void* loadModelThread(void *arg)
{
	int32 error = 0;

	char *name = arg;

	UUID modelName = idFromName(name);

	Model model = {};

	ASSET_LOG(MODEL, name, "Loading model (%s)...\n", name);

	model.name = modelName;
	model.lifetime = config.assetsConfig.minModelLifetime;

	char *modelFolder = getFullFilePath(name, NULL, "resources/models");

	char *assetFilename = getFullFilePath(name, "asset", modelFolder);
	FILE *assetFile = fopen(assetFilename, "rb");

	if (assetFile)
	{
		uint8 assetBinaryFileVersion;
		fread(&assetBinaryFileVersion, sizeof(uint8), 1, assetFile);

		if (assetBinaryFileVersion < ASSET_BINARY_FILE_VERSION)
		{
			ASSET_LOG(MODEL, name, "WARNING: %s out of date\n", assetFilename);
			error = -1;
		}
	}
	else
	{
		error = -1;
	}

	if (error != -1)
	{
		char *meshFilename = getFullFilePath(name, "mesh", modelFolder);
		FILE *meshFile = fopen(meshFilename, "rb");

		if (meshFile)
		{
			uint8 meshBinaryFileVersion;
			fread(&meshBinaryFileVersion, sizeof(uint8), 1, meshFile);

			if (meshBinaryFileVersion < MESH_BINARY_FILE_VERSION)
			{
				ASSET_LOG(
					MODEL,
					name,
					"WARNING: %s out of date\n",
					meshFilename);
				error = -1;
			}
		}
		else
		{
			error = -1;
		}

		if (error != -1)
		{
			fread(&model.numSubsets, sizeof(uint32), 1, assetFile);
			model.subsets = calloc(model.numSubsets, sizeof(Subset));

			for (uint32 i = 0; i < model.numSubsets; i++)
			{
				model.subsets[i].name = readStringAsUUID(assetFile);
			}

			char *masksFolder = getFullFilePath(
				"masks",
				NULL,
				modelFolder);

			error = loadMaskTexture(
				masksFolder,
				&model,
				'm',
				&model.materialTexture);

			if (error != -1)
			{
				error = loadMaskTexture(
					masksFolder,
					&model,
					'o',
					&model.opacityTexture);
			}

			free(masksFolder);

			if (error != -1)
			{
				for (uint32 i = 0; i < model.numSubsets; i++)
				{
					error = loadSubset(
						&model.subsets[i],
						assetFile,
						meshFile,
						name);

					if (error == -1)
					{
						break;
					}
				}

				if (error != -1)
				{
					error = loadAnimations(
						&model.numAnimations,
						&model.animations,
						&model.skeleton,
						meshFile);
				}
			}
		}
		else
		{
			ASSET_LOG(MODEL, name, "Failed to open %s\n", meshFilename);
			error = -1;
		}

		if (meshFile)
		{
			fclose(meshFile);
		}

		free(meshFilename);
	}
	else
	{
		ASSET_LOG(MODEL, name, "Failed to open %s\n", assetFilename);
		error = -1;
	}

	if (assetFile)
	{
		fclose(assetFile);
	}

	free(assetFilename);
	free(modelFolder);

	if (error != -1)
	{
		pthread_mutex_lock(&uploadModelsMutex);
		hashMapInsert(uploadModelsQueue, &modelName, &model);
		pthread_mutex_unlock(&uploadModelsMutex);

		ASSET_LOG(
			MODEL,
			name,
			"Successfully loaded model (%s)\n",
			name);
	}
	else
	{
		ASSET_LOG(MODEL, name, "Failed to load model (%s)\n", name);
	}

	ASSET_LOG_COMMIT(MODEL, name);

	pthread_mutex_lock(&loadingModelsMutex);
	hashMapDelete(loadingModels, &modelName);
	pthread_mutex_unlock(&loadingModelsMutex);

	free(name);

	EXIT_LOADING_THREAD;
}

void uploadModelToGPU(Model *model)
{
	LOG("Transferring model (%s) onto GPU...\n", model->name.string);

	for (uint32 i = 0; i < model->numSubsets; i++)
	{
		Subset *subset = &model->subsets[i];
		uploadMeshToGPU(&subset->mesh, subset->name.string);
	}

	LOG("Successfully transferred model (%s) onto GPU\n", model->name.string);
}

GET_ASSET_FUNCTION(
	model,
	models,
	Model,
	getModel(const char *name),
	idFromName(name));

void freeModelData(Model *model)
{
	LOG("Freeing model (%s)...\n", model->name.string);

	for (uint32 i = 0; i < model->numSubsets; i++)
	{
		freeMesh(&model->subsets[i].mesh);
	}

	free(model->subsets);

	freeAnimations(
		model->numAnimations,
		model->animations,
		&model->skeleton);

	LOG("Successfully freed model (%s)\n", model->name.string);
}

void swapMeshMaterial(
	const char *modelName,
	const char *meshName,
	const char *materialName)
{
	UUID name = idFromName(modelName);

	pthread_mutex_lock(&modelsMutex);
	Model *model = hashMapGetData(models, &name);

	if (model)
	{
		for (uint32 i = 0; i < model->numSubsets; i++)
		{
			Subset *subset = &model->subsets[i];
			if (!strcmp(subset->name.string, meshName))
			{
				Material *material = &subset->material;
				createMaterial(idFromName(materialName), material);

				pthread_mutex_unlock(&modelsMutex);
				return;
			}
		}
	}

	pthread_mutex_unlock(&modelsMutex);
}

int32 loadSubset(
	Subset *subset,
	FILE *assetFile,
	FILE *meshFile,
	const char *modelName)
{
	ASSET_LOG(
		MODEL,
		modelName,
		"Loading subset (%s)...\n",
		subset->name.string);

	if (loadMaterial(&subset->material, assetFile, modelName) == -1)
	{
		return -1;
	}

	loadMask(&subset->mask, assetFile, modelName);
	loadMesh(&subset->mesh, meshFile, modelName);

	ASSET_LOG(
		MODEL,
		modelName,
		"Successfully loaded subset (%s)\n",
		subset->name.string);

	return 0;
}