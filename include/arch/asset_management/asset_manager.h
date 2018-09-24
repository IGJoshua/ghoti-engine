#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#define MODELS_BUCKET_COUNT 1031
#define TEXTURES_BUCKET_COUNT 5003
#define MATERIAL_FOLDERS_BUCKET_COUNT 521
#define FONTS_BUCKET_COUNT 257
#define IMAGES_BUCKET_COUNT 1031
#define AUDIO_BUCKET_COUNT 257
#define PARTICLES_BUCKET_COUNT 1031

#define LOADING_MODELS_BUCKET_COUNT 521
#define LOADING_TEXTURES_BUCKET_COUNT 2503
#define LOADING_IMAGES_BUCKET_COUNT 521

#define UPLOAD_MODELS_BUCKET_COUNT 521
#define UPLOAD_TEXTURES_BUCKET_COUNT 2503
#define UPLOAD_IMAGES_BUCKET_COUNT 521

void initializeAssetManager(real64 *dt);
void setUpdateAssetManagerFlag(void);
void uploadAssets(void);
void freeAssets(void);
void shutdownAssetManager(void);