#pragma once
#include "defines.h"

#include "renderer/renderer_types.h"

#include <stdio.h>

int32 loadMaterial(Material *material, FILE *file, const char *modelName);
int32 createMaterial(UUID name, Material *material);
void loadMaterialFolders(UUID name);
int32 loadMaterialComponentTexture(
	UUID materialName,
	MaterialComponentType materialComponentType,
	UUID *textureName);
void freeMaterial(Material *material);