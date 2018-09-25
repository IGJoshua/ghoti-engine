#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

#include "core/log.h"

#include <IL/il.h>

void loadTexture(const char *filename, const char *name);
int32 loadTextureData(
	AssetLogType type,
	const char *typeName,
	const char *name,
	const char *filename,
	TextureFormat format,
	ILuint *devilID);
int32 uploadTextureToGPU(Texture *texture);
Texture getTexture(const char *name);
char* getFullTextureFilename(const char *filename);
void freeTexture(UUID name);
void freeTextureData(Texture *texture);