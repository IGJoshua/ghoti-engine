#include "asset_management/asset_manager_types.h"
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

extern Config config;

extern HashMap models;
extern pthread_mutex_t modelsMutex;

extern HashMap uploadModelsQueue;
extern pthread_mutex_t uploadModelsMutex;

extern uint32 assetThreadCount;
extern pthread_mutex_t assetThreadsMutex;
extern pthread_cond_t assetThreadsCondition;

internal void* acquireModelThread(void *arg);
internal void* loadModelThread(void *arg);

internal int32 loadSubset(
	Subset *subset,
	FILE *assetFile,
	FILE *meshFile,
	const char *modelName);

void loadModel(const char *name)
{
	char *modelName = calloc(1, strlen(name) + 1);
	strcpy(modelName, name);

	pthread_t acquisitionThread;
	pthread_create(
		&acquisitionThread,
		NULL,
		&acquireModelThread,
		(void*)modelName);
	pthread_detach(acquisitionThread);
}

void* acquireModelThread(void *arg)
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
	pthread_create(&loadingThread, NULL, &loadModelThread, arg);
	pthread_detach(loadingThread);

	EXIT_THREAD(NULL);
}

void* loadModelThread(void *arg)
{
	int32 error = 0;

	char *name = arg;

	UUID modelName = idFromName(name);

	pthread_mutex_lock(&modelsMutex);
	Model *modelResource = hashMapGetData(models, &modelName);

	if (!modelResource)
	{
		pthread_mutex_unlock(&modelsMutex);
		pthread_mutex_lock(&uploadModelsMutex);

		if (hashMapGetData(uploadModelsQueue, &modelName))
		{
			pthread_mutex_unlock(&uploadModelsMutex);
			error = 1;
		}

		pthread_mutex_unlock(&uploadModelsMutex);

		if (error != 1)
		{
			Model model = {};

			ASSET_LOG(MODEL, name, "Loading model (%s)...\n", name);

			model.name = modelName;
			model.refCount = 1;

			char *modelFolder = getFullFilePath(name, NULL, "resources/models");

			char *assetFilename = getFullFilePath(name, "asset", modelFolder);
			FILE *assetFile = fopen(assetFilename, "rb");

			if (assetFile)
			{
				char *meshFilename = getFullFilePath(name, "mesh", modelFolder);
				FILE *meshFile = fopen(meshFilename, "rb");

				if (meshFile)
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
		}
	}
	else
	{
		modelResource->refCount++;
		pthread_mutex_unlock(&modelsMutex);
	}

	ASSET_LOG_COMMIT(MODEL, name);
	free(name);

	pthread_mutex_lock(&assetThreadsMutex);
	assetThreadCount--;
	pthread_mutex_unlock(&assetThreadsMutex);
	pthread_cond_broadcast(&assetThreadsCondition);

	EXIT_THREAD(NULL);
}

void uploadModelToGPU(Model *model)
{
	LOG("Transferring model (%s) onto GPU...\n", model->name.string);

	for (uint32 i = 0; i < model->numSubsets; i++)
	{
		Subset *subset = &model->subsets[i];

		LOG("Transferring mesh (%s) onto GPU...\n", subset->name.string);

		uploadMeshToGPU(&subset->mesh);

		LOG("Successfully transferred mesh (%s) onto GPU\n",
				  subset->name.string);
	}

	LOG("Successfully transferred model (%s) onto GPU\n", model->name.string);
}

Model getModel(const char *name)
{
	Model model = {};

	if (strlen(name) > 0)
	{
		UUID nameID = idFromName(name);

		pthread_mutex_lock(&modelsMutex);
		Model *modelResource = hashMapGetData(models, &nameID);
		pthread_mutex_unlock(&modelsMutex);

		if (modelResource)
		{
			model = *modelResource;
		}
	}

	return model;
}

void freeModel(const char *name)
{
	UUID modelName = idFromName(name);

	pthread_mutex_lock(&modelsMutex);
	Model *model = hashMapGetData(models, &modelName);

	if (model)
	{
		freeTexture(model->materialTexture);
		freeTexture(model->opacityTexture);

		for (uint32 i = 0; i < model->numSubsets; i++)
		{
			Subset *subset = &model->subsets[i];
			freeMaterial(&subset->material);
			freeMask(&subset->mask);
		}

		model->refCount--;
	}

	pthread_mutex_unlock(&modelsMutex);
}

void freeModelData(Model *model)
{
	LOG("Freeing model data (%s)...\n", model->name.string);

	for (uint32 i = 0; i < model->numSubsets; i++)
	{
		freeMesh(&model->subsets[i].mesh);
	}

	free(model->subsets);

	freeAnimations(
		model->numAnimations,
		model->animations,
		&model->skeleton);

	LOG("Successfully freed model data (%s)\n", model->name.string);
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
				freeMaterial(material);
				createMaterial(idFromName(materialName), material);
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
