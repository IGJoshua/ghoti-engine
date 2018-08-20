#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

#include <IL/il.h>

int32 loadTexture(const char *filename, const char *name);
int32 loadTextureData(
	const char *filename,
	TextureFormat format,
	ILuint *devilID);
Texture* getTexture(const char *name);
char* getFullTextureFilename(const char *filename);
void freeTexture(UUID name);