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

extern HashMap textures;

int32 loadMask(Mask *mask, FILE *file)
{
	LOG("Loading masks...\n");

	loadMaterial(&mask->collectionMaterial, file);
	loadMaterial(&mask->grungeMaterial, file);
	loadMaterial(&mask->wearMaterial, file);
	fread(&mask->opacity, sizeof(real32), 1, file);

	LOG("Successfully loaded masks\n");

	return 0;
}

int32 loadMaskTexture(const char *masksFolder,
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
		if (loadTexture(fullFilename, textureName->string) == -1)
		{
			free(fullFilename);
			return -1;
		}

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