#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"
#include "renderer/renderer_types.h"

struct aiMesh;

int32 loadMesh(const struct aiMesh *mesh, MeshData *meshData);
int32 freeMesh(Mesh *mesh);
