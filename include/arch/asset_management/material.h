#pragma once
#include "defines.h"

#include "renderer/renderer_types.h"

#include <stdio.h>

int32 loadMaterial(Material *material, FILE *file);
int32 loadMaterialTextures(Material *material);
void freeMaterial(Material *material);