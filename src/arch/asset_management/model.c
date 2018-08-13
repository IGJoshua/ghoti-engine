#include "asset_management/model.h"
#include "asset_management/mesh.h"
#include "asset_management/material.h"
#include "asset_management/texture.h"

#include "renderer/shader.h"

#include "core/log.h"

#include "file/utilities.h"

#include "json-utilities/utilities.h"

#include "model-utility/material_exporter.h"

#include <malloc.h>
#include <string.h>

extern Model *models;
extern uint32 numModels;
extern uint32 modelsCapacity;

extern uint32 numTextures;
extern uint32 texturesCapacity;

internal int32 loadMaterials(Model *model);
internal int32 loadTextures(Model *model);

int32 loadModel(const char *name)
{
	Model *model = getModel(name);

	if (name && !model)
	{
		LOG("Loading model (%s)...\n", name);

		if (numModels + 1 > modelsCapacity)
		{
			increaseModelsCapacity();
		}

		model = &models[numModels++];

		model->name = malloc(strlen(name) + 1);
		strcpy(model->name, name);

		if (loadMaterials(model) == -1)
		{
			return -1;
		}

		if (loadMesh(model) == -1)
		{
			return -1;
		}

		LOG("Successfully loaded model (%s)\n", name);

		LOG("Model Count: %d\n", numModels);
		LOG("Models Capacity: %d\n", modelsCapacity);
	}

	if (model)
	{
		model->refCount++;
	}

	return 0;
}

void increaseModelsCapacity()
{
	modelsCapacity += RESOURCE_REALLOCATION_AMOUNT;

	uint32 previousBufferSize = numModels * sizeof(Model);
	uint32 newBufferSize = modelsCapacity * sizeof(Model);

	if (previousBufferSize == 0)
	{
		models = calloc(newBufferSize, 1);
	}
	else
	{
		models = realloc(models, newBufferSize);
		memset(
			models + previousBufferSize,
			0,
			newBufferSize - previousBufferSize);
	}

	LOG("Increased models capacity to %d to hold 1 new model\n",
		modelsCapacity);
}


int32 loadMaterials(Model *model)
{
	int32 error = 0;

	char *modelFolder = getFullFilePath(model->name, NULL, "resources/models");
	char *modelFilename = getFullFilePath(model->name, NULL, modelFolder);

	if (exportMaterials(modelFilename) == -1)
	{
		free(modelFilename);
		free(modelFolder);
		return -1;
	}

	free(modelFilename);

	char *assetFilename = getFullFilePath(model->name, NULL, modelFolder);

	if (exportAsset(assetFilename, LOG_FILE_NAME) == -1)
	{
		free(assetFilename);
		free(modelFolder);
		return -1;
	}

	free(assetFilename);

	assetFilename = getFullFilePath(model->name, "asset", modelFolder);
	free(modelFolder);

	FILE *file = fopen(assetFilename, "rb");

	if (file)
	{
		uint32 numMaterials;
		fread(&numMaterials, sizeof(uint32), 1, file);

		model->materials = calloc(numMaterials, sizeof(Material));

		for (uint32 i = 0; i < numMaterials; i++)
		{
			error = loadMaterial(
				&model->materials[model->numSubsets++],
				file);
			if (error == -1)
			{
				break;
			}
		}

		fclose(file);
	}
	else
	{
		LOG("Failed to open %s\n", assetFilename);
		error = -1;
	}

	free(assetFilename);

	if (error != -1)
	{
		error = loadTextures(model);
	}

	return error;
}

int32 loadTextures(Model *model)
{
	uint32 numUniqueTextures = 0;
	uint32 i, j;

	for (i = 0; i < model->numSubsets; i++)
	{
		Material *material = &model->materials[i];
		for (j = 0; j < MATERIAL_COMPONENT_TYPE_COUNT; j++)
		{
			MaterialComponent *materialComponent = &material->components[j];
			if (materialComponent->texture &&
				strlen(materialComponent->texture) > 0 &&
				!getTexture(materialComponent->texture))
			{
				numUniqueTextures++;
			}
		}
	}

	if (numTextures + numUniqueTextures > texturesCapacity)
	{
		increaseTexturesCapacity(numUniqueTextures);
	}

	for (i = 0; i < model->numSubsets; i++)
	{
		Material *material = &model->materials[i];
		for (j = 0; j < MATERIAL_COMPONENT_TYPE_COUNT; j++)
		{
			MaterialComponent *materialComponent = &material->components[j];

			if (materialComponent->texture &&
				strlen(materialComponent->texture) > 0)
			{
				if (loadTexture(materialComponent->texture, true, 0) == -1)
				{
					return -1;
				}
			}
		}
	}

	LOG("Texture Count: %d\n", numTextures);
	LOG("Textures Capacity: %d\n", texturesCapacity);

	return 0;
}

Model* getModel(const char *name)
{
	if (name && strlen(name) > 0)
	{
		for (uint32 i = 0; i < numModels; i++)
		{
			if (!strcmp(models[i].name, name))
			{
				return &models[i];
			}
		}
	}

	return NULL;
}

uint32 getModelIndex(const char *name)
{
	if (name && strlen(name) > 0)
	{
		for (uint32 i = 0; i < numModels; i++)
		{
			if (!strcmp(models[i].name, name))
			{
				return i;
			}
		}
	}

	return 0;
}

int32 freeModel(const char *name)
{
	Model *model = getModel(name);

	if (!model)
	{
		LOG("Could not find model\n");
		return -1;
	}

	uint32 index = getModelIndex(name);

	if (--(model->refCount) == 0)
	{
		LOG("Freeing model (%s)...\n", name);

		free(model->name);

		for (uint32 i = 0; i < model->numSubsets; i++)
		{
			if (freeMaterial(&model->materials[i]) == -1)
			{
				return -1;
			}

			if (freeMesh(&model->meshes[i]) == -1)
			{
				return -1;
			}
		}

		free(model->materials);
		free(model->meshes);

		numModels--;
		Model *resizedModels = calloc(modelsCapacity, sizeof(Model));
		memcpy(resizedModels, models, index * sizeof(Model));

		if (index < numModels)
		{
			memcpy(
				&resizedModels[index],
				&models[index + 1],
				(numModels - index) * sizeof(Model));
		}

		free(models);
		models = resizedModels;

		LOG("Successfully freed model (%s)\n", name);
		LOG("Model Count: %d\n", numModels);
	}

	return 0;
}
