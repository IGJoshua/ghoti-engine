#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

#include <IL/il.h>

int32 loadTexture(const char *name, bool genMipmaps, ILuint *dataOut);
int32 loadTextureWithFormat(
	const char *name,
	TextureFormat format,
	bool genMipmaps,
	ILuint *dataOut);
void increaseTexturesCapacity(uint32 amount);
Texture* getTexture(const char *name);
uint32 getTextureIndex(const char *name);
int32 freeTexture(const char *name);
