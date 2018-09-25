#include "asset_management/asset_manager_types.h"
#include "asset_management/mask.h"
#include "asset_management/material.h"
#include "asset_management/texture.h"

#include "core/log.h"

#include "file/utilities.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include <malloc.h>
#include <string.h>

void loadMask(Mask *mask, FILE *file, const char *modelName)
{
	ASSET_LOG(MODEL, modelName, "Loading masks...\n");

	loadMaterial(&mask->collectionMaterial, file, modelName);
	loadMaterial(&mask->grungeMaterial, file, modelName);
	loadMaterial(&mask->wearMaterial, file, modelName);
	fread(&mask->opacity, sizeof(real32), 1, file);

	ASSET_LOG(MODEL, modelName, "Successfully loaded masks\n");
}

int32 loadMaskTexture(
	const char *masksFolder,
	Model *model,
	char suffix,
	UUID *textureName)
{
	memset(textureName, 0, sizeof(UUID));
	sprintf(textureName->string, "%s_%c", model->name.string, suffix);

	char *filename = malloc(
		strlen(masksFolder) + strlen(textureName->string) + 5);
	sprintf(filename, "%s/mt_%s", masksFolder, textureName->string);

	char *fullFilename = getFullTextureFilename(filename);
	free(filename);

	if (fullFilename)
	{
		loadTexture(fullFilename, textureName->string);
		free(fullFilename);
	}

	return 0;
}

void freeMask(Mask *mask)
{
	freeMaterial(&mask->collectionMaterial);
	freeMaterial(&mask->grungeMaterial);
	freeMaterial(&mask->wearMaterial);
}