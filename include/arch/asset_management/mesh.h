#pragma once
#include "defines.h"

#include "asset_management/model.h"
#include "renderer/renderer_types.h"

int32 loadMesh(Model *model);
int32 freeMesh(Mesh *mesh);