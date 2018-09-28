#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

#include "core/log.h"

void loadTexture(const char *filename, const char *name);
int32 loadTextureData(
	AssetLogType type,
	const char *typeName,
	const char *name,
	const char *filename,
	int32 numComponents,
	TextureData *data);
int32 uploadTextureToGPU(
	const char *name,
	const char *type,
	GLuint *id,
	TextureData *data,
	bool textureFiltering,
	bool transparent);
Texture getTexture(const char *name);
char* getFullTextureFilename(const char *filename);
void freeTextureData(Texture *texture);