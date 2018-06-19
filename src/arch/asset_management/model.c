#include "asset_management/model.h"
#include "asset_management/mesh.h"
#include "asset_management/material.h"
#include "asset_management/texture.h"

#include "renderer/shader.h"

#include "file/utilities.h"

#include "json-utilities/utilities.h"

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <malloc.h>

extern Model *models;
extern uint32 numModels;
extern uint32 modelsCapacity;

extern uint32 numTextures;
extern uint32 texturesCapacity;

internal int32 loadMaterials(Model *model);
internal int32 loadTextures(Model *model);
internal int32 loadSubsets(Model *model);

int32 loadModel(const char *name)
{
	Model *model = getModel(name);

	if (name && !model)
	{
		printf("Loading model (%s)...\n", name);

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

		if (loadSubsets(model) == -1)
		{
			return -1;
		}

		printf("Successfully loaded model (%s)\n", name);
	}

	if (model)
	{
		printf("Model (%s) Reference Count: %d\n", name, ++(model->refCount));
	}

	printf("Model Count: %d\n", numModels);
	printf("Models Capacity: %d\n", modelsCapacity);

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

	printf(
		"Increased models capacity to %d to hold 1 new model\n",
		modelsCapacity);
}


int32 loadMaterials(Model *model)
{
	int32 error = 0;

	char *assetFolder = getFullFilePath(model->name, NULL, "resources/models");
	char *assetFilename = getFullFilePath(model->name, NULL, assetFolder);

	exportAsset(assetFilename);
	free(assetFilename);

	assetFilename = getFullFilePath(model->name, "asset", assetFolder);
	free(assetFolder);

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
		printf("Failed to open %s\n", assetFilename);
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
			if (
				materialComponent->texture &&
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

			if (loadTexture(materialComponent->texture) == -1)
			{
				return -1;
			}
		}
	}

	printf("Texture Count: %d\n", numTextures);
	printf("Textures Capacity: %d\n", texturesCapacity);

	return 0;
}

int32 loadSubsets(Model *model)
{
	char *modelFolder = getFullFilePath(model->name, NULL, "resources/models");
	char *modelFilename = getFullFilePath(model->name, "dae", modelFolder);
	free(modelFolder);

	const struct aiScene *scene = aiImportFile(
		modelFilename,
		aiProcessPreset_TargetRealtime_Quality &
		~aiProcess_SplitLargeMeshes);

	if (!scene) {
		printf("Failed to open %s\n", modelFilename);
		free(modelFilename);
		return -1;
	}

	free(modelFilename);

	model->meshes = calloc(model->numSubsets, sizeof(Mesh));

	for (uint32 i = 0; i < model->numSubsets; i++)
	{
		Material *material = &model->materials[i];

		for (uint32 j = 0; j < scene->mNumMeshes; j++)
		{
			struct aiMesh *mesh = scene->mMeshes[j];
			if (!strcmp(material->name, mesh->mName.data))
			{
				printf("Loading subset mesh (%s)...\n", material->name);
				if (loadMesh(mesh,
					&model->materials[i],
					&model->meshes[i]) == -1)
				{
					aiReleaseImport(scene);
					return -1;
				}
				printf(
					"Successfully loaded subset mesh (%s)\n",
					material->name);

				break;
			}
		}
	}

	aiReleaseImport(scene);

	return 0;
}

Model* getModel(const char *name)
{
	if (name)
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
	if (name)
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
	printf("Freeing model (%s)...\n", name);

	Model *model = getModel(name);

	if (!model)
	{
		printf("Could not find model\n");
		return -1;
	}

	uint32 index = getModelIndex(name);

	if (--(model->refCount) == 0)
	{
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

		printf("Successfully freed model (%s)\n", name);
		printf("Model Count: %d\n", numModels);
	}
	else
	{
		printf(
			"Successfully reduced model (%s) reference count to %d\n",
			name,
			model->refCount);
	}

	return 0;
}
