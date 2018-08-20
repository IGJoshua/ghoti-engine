#pragma once
#include "defines.h"

#include "renderer/renderer_types.h"

#include <stdio.h>

int32 loadMaterial(Material *material, FILE *file);
void freeMaterial(Material *material);