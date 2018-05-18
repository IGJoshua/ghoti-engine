#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"
#include "renderer/renderer_types.h"

struct aiScene;
struct aiMesh;

int32 loadMaterial(
	const struct aiScene *scene,
	const struct aiMesh *mesh,
	const MeshData *meshData,
	Material *material
);

int32 freeMaterial(Material *material);
