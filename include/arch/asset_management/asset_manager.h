#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#define MODELS_BUCKET_COUNT 1031
#define TEXTURES_BUCKET_COUNT 5501
#define MATERIAL_FOLDERS_BUCKET_COUNT 521
#define FONTS_BUCKET_COUNT 257

void initializeAssetManager(void);
void loadAssets(UUID componentID, ComponentDataEntry *entry);
void shutdownAssetManager(void);
