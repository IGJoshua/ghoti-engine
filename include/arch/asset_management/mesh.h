#pragma once
#include "defines.h"

#include <assimp/mesh.h>

#include "renderer/renderer_types.h"

int32 loadMesh(
	const struct aiMesh *meshData,
	const Material *material,
	Mesh *mesh);
int32 freeMesh(Mesh *mesh);
