#include "asset_management/asset_manager.h"
#include "asset_management/asset_manager_types.h"
#include "asset_management/audio.h"
#include "asset_management/cubemap.h"
#include "asset_management/font.h"
#include "asset_management/image.h"
#include "asset_management/model.h"
#include "asset_management/particle.h"
#include "asset_management/texture.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include <string.h>
#include <pthread.h>

#define INTERNAL_ASSET_VARIABLES(Assets) \
internal List free ## Assets ## Queue; \
internal pthread_mutex_t free ## Assets ## Mutex

#define ASSET_VARIABLES(assets, Assets) \
EXTERN_ASSET_VARIABLES(assets, Assets); \
INTERNAL_ASSET_VARIABLES(Assets)

ASSET_VARIABLES(models, Models);
ASSET_VARIABLES(textures, Textures);
ASSET_VARIABLES(fonts, Fonts);
ASSET_VARIABLES(images, Images);
ASSET_VARIABLES(audioFiles, Audio);
ASSET_VARIABLES(particles, Particles);
ASSET_VARIABLES(cubemaps, Cubemaps);

extern HashMap materialFolders;
extern pthread_mutex_t materialFoldersMutex;

EXTERN_ASSET_MANAGER_VARIABLES;

extern pthread_mutex_t stbImageFlipMutex;

internal pthread_t assetManagerThread;

internal bool exitAssetManagerThread;
internal pthread_mutex_t exitAssetManagerMutex;

internal bool updateAssetManagerFlag;
internal pthread_mutex_t updateAssetManagerMutex;
internal pthread_cond_t updateAssetManagerCondition;

internal void* updateAssetManager(void *arg);

#define INITIALIZE_ASSET(assets, Asset, Assets, ASSET) \
assets = createHashMap( \
	sizeof(UUID), \
	sizeof(Asset), \
	ASSET ## _BUCKET_COUNT, \
	(ComparisonOp)&strcmp); \
pthread_mutex_init(&assets ## Mutex, NULL); \
\
loading ## Assets = createHashMap( \
	sizeof(UUID), \
	sizeof(bool), \
	LOADING_ ## ASSET ## _BUCKET_COUNT, \
	(ComparisonOp)&strcmp); \
pthread_mutex_init(&loading ## Assets ## Mutex, NULL); \
\
upload ## Assets ## Queue = createHashMap( \
	sizeof(UUID), \
	sizeof(Asset), \
	UPLOAD_ ## ASSET ## _BUCKET_COUNT, \
	(ComparisonOp)&strcmp); \
pthread_mutex_init(&upload ## Assets ## Mutex, NULL); \
\
free ## Assets ## Queue = createList(sizeof(Asset)); \
pthread_mutex_init(&free ## Assets ## Mutex, NULL)

#define UPDATE_ASSET(asset, assets, Asset, Assets, ASSET, assetName) \
pthread_mutex_lock(&assets ## Mutex); \
\
for (HashMapIterator itr = hashMapGetIterator(assets); \
	 !hashMapIteratorAtEnd(itr);) \
{ \
	Asset *asset = hashMapIteratorGetValue(itr); \
\
	asset->lifetime -= dt; \
	if (asset->lifetime <= 0.0) \
	{ \
		pthread_mutex_lock(&free ## Assets ## Mutex); \
		listPushBack(&free ## Assets ## Queue, asset); \
		pthread_mutex_unlock(&free ## Assets ## Mutex); \
\
		UUID asset ## Name = asset->name; \
\
		hashMapMoveIterator(&itr); \
		hashMapDelete(assets, &asset ## Name); \
\
		ASSET_LOG( \
			ASSET, \
			asset ## Name.string, \
			"%s queued to be freed (%s)\n", \
			assetName, \
			asset ## Name.string); \
		ASSET_LOG( \
			ASSET, \
			asset ## Name.string, \
			"%s Count: %d\n", \
			assetName, \
			assets->count); \
		ASSET_LOG_COMMIT(ASSET, asset ## Name.string); \
	} \
	else \
	{ \
		hashMapMoveIterator(&itr); \
	} \
} \
\
pthread_mutex_unlock(&assets ## Mutex)

#define UPLOAD_ASSET(asset, assets, Asset, Assets, assetName, uploadFunction) \
pthread_mutex_lock(&upload ## Assets ## Mutex); \
\
for (HashMapIterator itr = hashMapGetIterator(upload ## Assets ## Queue); \
	 !hashMapIteratorAtEnd(itr);) \
{ \
	Asset *asset = hashMapIteratorGetValue(itr); \
\
	pthread_mutex_unlock(&upload ## Assets ## Mutex); \
	uploadFunction; \
	pthread_mutex_lock(&upload ## Assets ## Mutex); \
\
	UUID asset ## Name = asset->name; \
\
	pthread_mutex_lock(&assets ## Mutex); \
\
	hashMapInsert(assets, &asset ## Name, asset); \
	LOG("%s Count: %d\n", assetName, assets->count); \
\
	pthread_mutex_unlock(&assets ## Mutex); \
\
	hashMapMoveIterator(&itr); \
	hashMapDelete(upload ## Assets ## Queue, &asset ## Name ); \
} \
\
pthread_mutex_unlock(&upload ## Assets ## Mutex)

#define FREE_ASSET(asset, Asset, Assets) \
pthread_mutex_lock(&free ## Assets ## Mutex); \
\
for (ListIterator listItr = listGetIterator(&free ## Assets ## Queue); \
	 !listIteratorAtEnd(listItr);) \
{ \
	Asset *asset = LIST_ITERATOR_GET_ELEMENT(Asset, listItr); \
\
	pthread_mutex_unlock(&free ## Assets ## Mutex); \
	free ## Asset ## Data(asset); \
	pthread_mutex_lock(&free ## Assets ## Mutex); \
\
	listRemove(&free ## Assets ## Queue, &listItr); \
} \
\
pthread_mutex_unlock(&free ## Assets ## Mutex)

#define DESTROY_ASSET(asset, assets, Asset, Assets) \
for (ListIterator listItr = listGetIterator(&free ## Assets ## Queue); \
	 !listIteratorAtEnd(listItr); \
	 listMoveIterator(&listItr)) \
{ \
	Asset *asset = LIST_ITERATOR_GET_ELEMENT(Asset, listItr); \
	hashMapInsert(assets, &asset->name, asset); \
} \
\
listClear(&free ## Assets ## Queue); \
pthread_mutex_destroy(&free ## Assets ## Mutex); \
\
for (HashMapIterator itr = hashMapGetIterator(upload ## Assets ## Queue); \
	 !hashMapIteratorAtEnd(itr); \
	 hashMapMoveIterator(&itr)) \
{ \
	Asset *asset = hashMapIteratorGetValue(itr); \
	hashMapInsert(assets, &asset->name, asset); \
} \
\
freeHashMap(&upload ## Assets ## Queue); \
pthread_mutex_destroy(&upload ## Assets ## Mutex); \
\
freeHashMap(&loading ## Assets); \
pthread_mutex_destroy(&loading ## Assets ## Mutex); \
\
for (HashMapIterator itr = hashMapGetIterator(assets); \
	 !hashMapIteratorAtEnd(itr); \
	 hashMapMoveIterator(&itr)) \
{ \
	free ## Asset ## Data(hashMapIteratorGetValue(itr)); \
} \
\
freeHashMap(&assets); \
pthread_mutex_destroy(&assets ## Mutex)

void initializeAssetManager(real64 *dt) {
	INITIALIZE_ASSET(models, Model, Models, MODELS);
	INITIALIZE_ASSET(textures, Texture, Textures, TEXTURES);
	INITIALIZE_ASSET(fonts, Font, Fonts, FONTS);
	INITIALIZE_ASSET(images, Image, Images, IMAGES);
	INITIALIZE_ASSET(audioFiles, AudioFile, Audio, AUDIO);
	INITIALIZE_ASSET(particles, Particle, Particles, PARTICLES);
	INITIALIZE_ASSET(cubemaps, Cubemap, Cubemaps, CUBEMAPS);

	materialFolders = createHashMap(
		sizeof(UUID),
		sizeof(List),
		MATERIAL_FOLDERS_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&materialFoldersMutex, NULL);

	assetThreadCount = 0;
	pthread_mutex_init(&assetThreadsMutex, NULL);
	pthread_cond_init(&assetThreadsCondition, NULL);

	assetManagerIsShutdown = false;
	pthread_mutex_init(&assetManagerShutdownMutex, NULL);

	pthread_mutex_init(&stbImageFlipMutex, NULL);

	exitAssetManagerThread = false;
	pthread_mutex_init(&exitAssetManagerMutex, NULL);

	updateAssetManagerFlag = false;
	pthread_mutex_init(&updateAssetManagerMutex, NULL);
	pthread_cond_init(&updateAssetManagerCondition, NULL);

	pthread_create(&assetManagerThread, NULL, &updateAssetManager, dt);
}

uint32 getAssetThreadCount(void)
{
	uint32 count = 0;
	pthread_mutex_lock(&assetThreadsMutex);
	count = assetThreadCount;
	pthread_mutex_unlock(&assetThreadsMutex);
	return count;
}

void setUpdateAssetManagerFlag(void)
{
	pthread_mutex_lock(&updateAssetManagerMutex);
	updateAssetManagerFlag = true;
	pthread_mutex_unlock(&updateAssetManagerMutex);
	pthread_cond_signal(&updateAssetManagerCondition);
}

void* updateAssetManager(void *arg)
{
	real64 dt = *(real64*)arg;

	while (true)
	{
		pthread_mutex_lock(&exitAssetManagerMutex);

		if (exitAssetManagerThread)
		{
			pthread_mutex_unlock(&exitAssetManagerMutex);
			break;
		}

		pthread_mutex_unlock(&exitAssetManagerMutex);

		UPDATE_ASSET(model, models, Model, Models, MODEL, "Model");
		UPDATE_ASSET(texture, textures, Texture, Textures, TEXTURE, "Texture");
		UPDATE_ASSET(font, fonts, Font, Fonts, FONT, "Font");
		UPDATE_ASSET(image, images, Image, Images, IMAGE, "Image");
		UPDATE_ASSET(audio, audioFiles, AudioFile, Audio, AUDIO, "Audio");
		UPDATE_ASSET(
			particle,
			particles,
			Particle,
			Particles,
			PARTICLE,
			"Particle");
		UPDATE_ASSET(cubemap, cubemaps, Cubemap, Cubemaps, CUBEMAP, "Cubemap");

		pthread_mutex_lock(&updateAssetManagerMutex);

		while (!updateAssetManagerFlag)
		{
			pthread_cond_wait(
				&updateAssetManagerCondition,
				&updateAssetManagerMutex);
		}

		updateAssetManagerFlag = false;

		pthread_mutex_unlock(&updateAssetManagerMutex);
	}

	EXIT_THREAD(NULL);
}

void uploadAssets(void)
{
	UPLOAD_ASSET(
		model,
		models,
		Model,
		Models,
		"Model",
		uploadModelToGPU(model));

	UPLOAD_ASSET(
		texture,
		textures,
		Texture,
		Textures,
		"Texture",
		uploadTextureToGPU(
			texture->name.string,
			"texture",
			&texture->id,
			&texture->data,
			true,
			false));

	UPLOAD_ASSET(
		font,
		fonts,
		Font,
		Fonts,
		"Font",
		uploadFontToGPU(font));

	UPLOAD_ASSET(
		image,
		images,
		Image,
		Images,
		"Image",
		uploadTextureToGPU(
			image->name.string,
			"image",
			&image->id,
			&image->data,
			image->textureFiltering,
			true));

	UPLOAD_ASSET(
		audio,
		audioFiles,
		AudioFile,
		Audio,
		"Audio",
		uploadAudioToSoundCard(audio));

	UPLOAD_ASSET(
		particle,
		particles,
		Particle,
		Particles,
		"Particle",
		uploadTextureToGPU(
			particle->name.string,
			"particle",
			&particle->id,
			&particle->data,
			particle->textureFiltering,
			true));

	UPLOAD_ASSET(
		cubemap,
		cubemaps,
		Cubemap,
		Cubemaps,
		"Cubemap",
		uploadCubemapToGPU(cubemap));
}

void freeAssets(void)
{
	FREE_ASSET(model, Model, Models);
	FREE_ASSET(texture, Texture, Textures);
	FREE_ASSET(font, Font, Fonts);
	FREE_ASSET(image, Image, Images);
	FREE_ASSET(audio, AudioFile, Audio);
	FREE_ASSET(particle, Particle, Particles);
	FREE_ASSET(cubemap, Cubemap, Cubemaps);
}

void shutdownAssetManager(void)
{
	pthread_mutex_lock(&assetManagerShutdownMutex);
	assetManagerIsShutdown = true;
	pthread_mutex_unlock(&assetManagerShutdownMutex);

	pthread_mutex_lock(&exitAssetManagerMutex);
	exitAssetManagerThread = true;
	pthread_mutex_unlock(&exitAssetManagerMutex);

	setUpdateAssetManagerFlag();

	pthread_join(assetManagerThread, NULL);

	pthread_mutex_destroy(&exitAssetManagerMutex);

	pthread_mutex_destroy(&updateAssetManagerMutex);
	pthread_cond_destroy(&updateAssetManagerCondition);

	pthread_mutex_lock(&assetThreadsMutex);

	while (assetThreadCount > 0)
	{
		pthread_cond_wait(&assetThreadsCondition, &assetThreadsMutex);
	}

	pthread_mutex_unlock(&assetThreadsMutex);

	pthread_mutex_destroy(&assetThreadsMutex);
	pthread_cond_destroy(&assetThreadsCondition);

	pthread_mutex_destroy(&assetManagerShutdownMutex);

	pthread_mutex_destroy(&stbImageFlipMutex);

	DESTROY_ASSET(model, models, Model, Models);
	DESTROY_ASSET(texture, textures, Texture, Textures);
	DESTROY_ASSET(font, fonts, Font, Fonts);
	DESTROY_ASSET(image, images, Image, Images);
	DESTROY_ASSET(audio, audioFiles, AudioFile, Audio);
	DESTROY_ASSET(particle, particles, Particle, Particles);
	DESTROY_ASSET(cubemap, cubemaps, Cubemap, Cubemaps);

	for (HashMapIterator itr = hashMapGetIterator(materialFolders);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		List *materialFoldersList = (List*)hashMapIteratorGetValue(itr);
		for (ListIterator listItr = listGetIterator(materialFoldersList);
			 !listIteratorAtEnd(listItr);
			 listMoveIterator(&listItr))
		{
			free(LIST_ITERATOR_GET_ELEMENT(MaterialFolder, listItr)->folder);
		}

		listClear(materialFoldersList);
	}

	freeHashMap(&materialFolders);
	pthread_mutex_destroy(&materialFoldersMutex);
}