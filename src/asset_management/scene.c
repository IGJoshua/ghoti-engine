#include "asset_management/scene.h"
#include "asset_management/model.h"

// TODO Add JSON loading
// #include <cjson/cJSON.h>

#include <malloc.h>
#include <string.h>

extern Model *models;
extern uint32 numModels;

/*int32 loadScene(const char *name)
{
	// TODO Get number of models in the scene from JSON file,
	// check for only models that haven't already been loaded
	// If model has already been loaded, increase refcount
	uint32 numSceneModels = 1;

	char **modelNames = malloc(numSceneModels * sizeof(char*));
	for (uint32 i = 0; i < numSceneModels; i++)
	{
		// TODO Get unique model names from JSON file
		modelNames[i] = "test";
	}

	uint32 previousBufferSize = numModels * sizeof(Model);
	uint32 newBufferSize = (numModels + numSceneModels) * sizeof(Model);

	if (previousBufferSize == 0)
	{
		models = malloc(newBufferSize);
	}
	else
	{
		models = realloc(models, newBufferSize);
	}

	for (uint32 i = 0; i < numSceneModels; i++)
	{
		loadModel(modelNames[i]);
	}

	return 0;
}

int32 unloadScene(Scene **scene)
{
	for (uint32 i = 0; i < (*scene)->numModels; i++)
	{
		freeModel((*scene)->models[i]);
	}

	free((*scene)->models);

	free(*scene);
	*scene = 0;

	return 0;
}*/
