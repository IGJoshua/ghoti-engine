#pragma once
#include "defines.h"

#include "renderer/renderer_types.h"

#include <stdio.h>

void loadMesh(Mesh *mesh, FILE *file, const char *modelName);
void uploadMeshToGPU(Mesh *mesh);
void freeMesh(Mesh *mesh);