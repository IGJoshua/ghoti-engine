#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

#define MESH_BINARY_FILE_VERSION 1

void loadModel(const char *name);
void uploadModelToGPU(Model *model);
Model getModel(const char *name);
void freeModelData(Model *model);

void swapMeshMaterial(
	const char *modelName,
	const char *meshName,
	const char *materialName);