#include "asset_management/asset_manager_types.h"
#include "asset_management/model.h"
#include "asset_management/material.h"
#include "asset_management/mask.h"
#include "asset_management/mesh.h"
#include "asset_management/animation.h"
#include "asset_management/texture.h"

#include "core/log.h"

#include "file/utilities.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include "ECS/scene.h"

extern HashMap models;

internal int32 loadSubset(Subset *subset, FILE *assetFile, FILE *meshFile);

int32 loadModel(const char *name)
{
	int32 error = 0;

	Model *modelResource = getModel(name);
	if (!modelResource)
	{
		Model model = {};

		ASSET_LOG("Loading model (%s)...\n", name);

		model.name = idFromName(name);
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

				char *masksFolder = getFullFilePath("masks", NULL, modelFolder);

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
							meshFile);

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
				ASSET_LOG("Failed to open %s\n", meshFilename);
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
			ASSET_LOG("Failed to open %s\n", assetFilename);
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
			// Lock mutex
			hashMapInsert(models, &model.name, &model);

			ASSET_LOG("Successfully loaded model (%s)\n", name);
			ASSET_LOG("Model Count: %d\n", models->count);
			// Unlock mutex
		}
		else
		{
			ASSET_LOG("Failed to load model (%s)\n", name);
		}
	}
	else
	{
		modelResource->refCount++;
	}

	return error;
}

void uploadModelToGPU(Model *model)
{
	ASSET_LOG("Transferring model (%s) onto GPU...\n", model->name.string);

	for (uint32 i = 0; i < model->numSubsets; i++)
	{
		Subset *subset = &model->subsets[i];

		ASSET_LOG("Transferring mesh (%s) onto GPU...\n", subset->name.string);

		uploadMeshToGPU(&subset->mesh);

		ASSET_LOG("Successfully transferred mesh (%s) onto GPU\n",
				  subset->name.string);
	}

	ASSET_LOG("Successfully transferred model (%s) onto GPU\n", model->name.string);
}

Model* getModel(const char *name)
{
	Model *model = NULL;
	if (strlen(name) > 0)
	{
		UUID nameID = idFromName(name);

		// Lock mutex
		model = hashMapGetData(models, &nameID);
		// Unlock mutex
	}

	return model;
}

void freeModel(const char *name)
{
	Model *model = getModel(name);
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
}

void freeModelData(Model *model)
{
	UUID modelName = model->name;

	ASSET_LOG("Freeing model (%s)...\n", modelName.string);

	for (uint32 i = 0; i < model->numSubsets; i++)
	{
		freeMesh(&model->subsets[i].mesh);
	}

	free(model->subsets);

	freeAnimations(
		model->numAnimations,
		model->animations,
		&model->skeleton);

	// Lock mutex
	hashMapDelete(models, &modelName);

	ASSET_LOG("Successfully freed model (%s)\n", modelName.string);
	ASSET_LOG("Model Count: %d\n", models->count);
	// Unlock mutex
}

void swapMeshMaterial(
	const char *modelName,
	const char *meshName,
	const char *materialName)
{
	Model *model = getModel(modelName);
	if (!model)
	{
		return;
	}

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

int32 loadSubset(Subset *subset, FILE *assetFile, FILE *meshFile)
{
	ASSET_LOG("Loading subset (%s)...\n", subset->name.string);

	if (loadMaterial(&subset->material, assetFile) == -1)
	{
		return -1;
	}

	loadMask(&subset->mask, assetFile);
	loadMesh(&subset->mesh, meshFile);

	ASSET_LOG("Successfully loaded subset (%s)\n", subset->name.string);

	return 0;
}