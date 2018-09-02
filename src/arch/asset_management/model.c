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
extern HashMap textures;

internal int32 loadSubset(Subset *subset, FILE *assetFile, FILE *meshFile);
internal void freeSubset(Subset *subset);

internal void deleteModel(const char *name);

int32 loadModel(const char *name)
{
	int32 error = 0;

	Model *modelResource = getModel(name);
	if (!modelResource)
	{
		Model model = {};

		LOG("Loading model (%s)...\n", name);

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
				LOG("Failed to open %s\n", meshFilename);
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
			LOG("Failed to open %s\n", assetFilename);
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
			hashMapInsert(models, &model.name, &model);

			LOG("Successfully loaded model (%s)\n", name);
			LOG("Model Count: %d\n", models->count);
		}
		else
		{
			LOG("Failed to load model (%s)\n", name);
		}
	}
	else
	{
		modelResource->refCount++;
	}

	return error;
}

Model* getModel(const char *name)
{
	Model *model = NULL;
	if (strlen(name) > 0)
	{
		UUID nameID = idFromName(name);
		model = hashMapGetData(models, &nameID);
	}

	return model;
}

void deleteModel(const char *name)
{
	UUID nameID = idFromName(name);
	hashMapDelete(models, &nameID);
}

void freeModel(const char *name)
{
	Model *model = getModel(name);
	if (model)
	{
		if (--model->refCount == 0)
		{
			LOG("Freeing model (%s)...\n", name);

			freeTexture(model->materialTexture);
			freeTexture(model->opacityTexture);

			for (uint32 i = 0; i < model->numSubsets; i++)
			{
				freeSubset(&model->subsets[i]);
			}

			free(model->subsets);

			freeAnimations(
				model->numAnimations,
				model->animations,
				&model->skeleton);

			deleteModel(name);

			LOG("Successfully freed model (%s)\n", name);
			LOG("Model Count: %d\n", models->count);
		}
	}
}

int32 loadSubset(Subset *subset, FILE *assetFile, FILE *meshFile)
{
	LOG("Loading subset (%s)...\n", subset->name.string);

	if (loadMaterial(&subset->material, assetFile) == -1)
	{
		return -1;
	}

	if (loadMask(&subset->mask, assetFile) == -1)
	{
		return -1;
	}

	if (loadMesh(&subset->mesh, meshFile) == -1)
	{
		return -1;
	}

	LOG("Successfully loaded subset (%s)\n", subset->name.string);

	return 0;
}

void freeSubset(Subset *subset)
{
	freeMesh(&subset->mesh);
	freeMaterial(&subset->material);
	freeMask(&subset->mask);
}
