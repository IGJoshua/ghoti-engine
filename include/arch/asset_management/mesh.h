#pragma once
#include "defines.h"

#include "renderer/renderer_types.h"

#include <stdio.h>

int32 loadMesh(Mesh *mesh, FILE *file);
void freeMesh(Mesh *mesh);