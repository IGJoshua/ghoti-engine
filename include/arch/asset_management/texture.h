#pragma once
#include "defines.h"

#include <assimp/cimport.h>

#include "asset_management/asset_manager_types.h"
#include "renderer/renderer_types.h"

int32 loadTexture(const struct aiString *name, TextureType type);
Texture* getTexture(const char *name);
uint32 getTextureIndex(const char *name);
int32 freeTexture(const char *name);
