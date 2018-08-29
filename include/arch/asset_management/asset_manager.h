#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#define MODELS_BUCKET_COUNT 1031
#define TEXTURES_BUCKET_COUNT 5501
#define MATERIAL_FOLDERS_BUCKET_COUNT 521
#define AUDIO_BUCKET_COUNT 101

void initializeAssetManager(void);
void loadAssets(UUID componentID, ComponentDataEntry *entry);
void freeAssets(UUID componentID, ComponentDataEntry *entry);
void shutdownAssetManager(void);
