#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#define EXTERN_ASSET_VARIABLES(assets, Assets) \
extern HashMap assets; \
extern pthread_mutex_t assets ## Mutex; \
\
extern HashMap loading ## Assets; \
extern pthread_mutex_t loading ## Assets ## Mutex; \
\
extern HashMap upload ## Assets ## Queue; \
extern pthread_mutex_t upload ## Assets ## Mutex

#define EXTERN_ASSET_MANAGER_VARIABLES \
extern uint32 assetThreadCount; \
extern pthread_mutex_t assetThreadsMutex; \
extern pthread_cond_t assetThreadsCondition; \
\
extern bool assetManagerIsShutdown; \
extern pthread_mutex_t assetManagerShutdownMutex

#define INTERNAL_ASSET_THREAD_VARIABLES(Asset) \
internal void* acquire ## Asset ## Thread(void *arg); \
internal void* load ## Asset ## Thread(void *arg)

#define START_ACQUISITION_THREAD(asset, Asset, Assets, arg, name) \
pthread_mutex_lock(&assetManagerShutdownMutex); \
\
if (assetManagerIsShutdown) \
{ \
	pthread_mutex_unlock(&assetManagerShutdownMutex); \
	return; \
} \
\
pthread_mutex_unlock(&assetManagerShutdownMutex); \
\
bool loading = true; \
pthread_mutex_lock(&loading ## Assets ## Mutex); \
hashMapInsert(loading ## Assets, &name, &loading); \
pthread_mutex_unlock(&loading ## Assets ## Mutex); \
\
pthread_t acquisitionThread; \
pthread_create( \
	&acquisitionThread, \
	NULL, \
	&acquire ## Asset ## Thread, \
	(void*)arg); \
pthread_detach(acquisitionThread)

#define ACQUISITION_THREAD(Asset) \
void* acquire ## Asset ## Thread(void *arg) \
{ \
	pthread_mutex_lock(&assetThreadsMutex); \
\
	while (assetThreadCount == config.assetsConfig.maxThreadCount) \
	{ \
		pthread_cond_wait(&assetThreadsCondition, &assetThreadsMutex); \
	} \
\
	assetThreadCount++; \
\
	pthread_cond_broadcast(&assetThreadsCondition); \
	pthread_mutex_unlock(&assetThreadsMutex); \
\
	pthread_t loadingThread; \
	pthread_create(&loadingThread, NULL, &load ## Asset ## Thread, arg); \
	pthread_detach(loadingThread); \
\
	EXIT_THREAD(NULL); \
}

#define EXIT_LOADING_THREAD \
pthread_mutex_lock(&assetThreadsMutex); \
assetThreadCount--; \
pthread_cond_broadcast(&assetThreadsCondition); \
pthread_mutex_unlock(&assetThreadsMutex); \
\
EXIT_THREAD(NULL)

#define GET_ASSET_FUNCTION( \
	asset, \
	assets, \
	Asset, \
	functionSignature, \
	getNameFunction) \
Asset functionSignature \
{ \
	Asset asset = {}; \
\
	if (strlen(name) > 0) \
	{ \
		UUID asset ## Name = getNameFunction; \
\
		pthread_mutex_lock(&assets ## Mutex); \
\
		Asset *asset ## Resource = hashMapGetData(assets, &asset ## Name); \
		if (asset ## Resource) \
		{ \
			asset ## Resource->lifetime = \
				config.assetsConfig.min ## Asset ## Lifetime; \
			asset = *asset ## Resource; \
		} \
\
		pthread_mutex_unlock(&assets ## Mutex); \
	} \
\
	return asset; \
}

#define MODELS_BUCKET_COUNT 1031
#define TEXTURES_BUCKET_COUNT 5003
#define MATERIAL_FOLDERS_BUCKET_COUNT 521
#define FONTS_BUCKET_COUNT 257
#define IMAGES_BUCKET_COUNT 1031
#define AUDIO_BUCKET_COUNT 257
#define PARTICLES_BUCKET_COUNT 1031
#define CUBEMAPS_BUCKET_COUNT 13

#define LOADING_MODELS_BUCKET_COUNT 521
#define LOADING_TEXTURES_BUCKET_COUNT 2503
#define LOADING_FONTS_BUCKET_COUNT 127
#define LOADING_IMAGES_BUCKET_COUNT 521
#define LOADING_AUDIO_BUCKET_COUNT 127
#define LOADING_PARTICLES_BUCKET_COUNT 521
#define LOADING_CUBEMAPS_BUCKET_COUNT 5

#define UPLOAD_MODELS_BUCKET_COUNT 521
#define UPLOAD_TEXTURES_BUCKET_COUNT 2503
#define UPLOAD_FONTS_BUCKET_COUNT 127
#define UPLOAD_IMAGES_BUCKET_COUNT 521
#define UPLOAD_AUDIO_BUCKET_COUNT 127
#define UPLOAD_PARTICLES_BUCKET_COUNT 521
#define UPLOAD_CUBEMAPS_BUCKET_COUNT 5

void initializeAssetManager(real64 *dt);
uint32 getAssetThreadCount(void);
void setUpdateAssetManagerFlag(void);
void uploadAssets(void);
void freeAssets(void);
void shutdownAssetManager(void);