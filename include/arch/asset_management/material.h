#pragma once
#include "defines.h"

#include "renderer/renderer_types.h"

#include <stdio.h>

int32 loadMaterial(Material *material, FILE *file);
void loadMaterialFolders(UUID name);
int32 loadMaterialComponentTexture(
	UUID materialName,
	MaterialComponentType materialComponentType,
	UUID *textureName);
void freeMaterial(Material *material);