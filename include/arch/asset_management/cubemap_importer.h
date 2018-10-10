#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

#define NUM_QUAD_VERTEX_ATTRIBUTES 2

typedef struct quad_vertex_t
{
	kmVec2 position;
	kmVec2 uv;
} QuadVertex;

void initializeCubemapImporter(void);
void importCubemap(Cubemap *cubemap);
void shutdownCubemapImporter(void);