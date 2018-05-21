#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"
#include "renderer/renderer_types.h"

struct aiMaterial;

int32 loadMaterial(
	const struct aiMaterial *materialData,
	Material *material
);

int32 freeMaterial(Material *material);
