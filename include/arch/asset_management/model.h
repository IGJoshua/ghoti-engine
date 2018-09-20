#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

int32 loadModel(const char *name);
void uploadModelToGPU(Model *model);
Model* getModel(const char *name);
void freeModel(const char *name);
void freeModelData(Model *model);

void swapMeshMaterial(
	const char *modelName,
	const char *meshName,
	const char *materialName);