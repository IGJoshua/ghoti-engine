#pragma once
#include "renderer_types.h"

struct aiScene;
struct aiMesh;

int32 loadMesh(const struct aiScene *scene, Mesh **m, const char *meshName, const struct aiMesh **out);
void freeMesh(Mesh **m);

int32 loadMaterials(const struct aiScene *scene, struct aiMesh *mesh, Material **mats, uint32 *numMaterials);

void renderMesh(Mesh *m);
