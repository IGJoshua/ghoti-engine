#pragma once
#include "renderer_types.h"

int32 loadMesh(Mesh **m, const char *filename);
void freeMesh(Mesh **m);

void renderMesh(Mesh *m);
