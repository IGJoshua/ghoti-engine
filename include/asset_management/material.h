#pragma once
#include "defines.h"

#include <assimp/material.h>

#include "renderer/renderer_types.h"

int32 loadMaterial(
	const struct aiMaterial *materialData,
	Material *material);
int32 freeMaterial(Material *material);
