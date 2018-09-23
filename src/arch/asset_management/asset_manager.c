#include "asset_management/asset_manager.h"
#include "asset_management/asset_manager_types.h"
#include "asset_management/model.h"
#include "asset_management/texture.h"
#include "asset_management/font.h"

#include "components/component_types.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include <string.h>
#include <pthread.h>

extern Config config;

extern HashMap models;
extern pthread_mutex_t modelsMutex;

extern HashMap textures;
extern pthread_mutex_t texturesMutex;

extern HashMap materialFolders;
extern HashMap fonts;
extern HashMap images;
extern HashMap particles;

extern HashMap uploadModelsQueue;
extern pthread_mutex_t uploadModelsMutex;

extern HashMap uploadTexturesQueue;
extern pthread_mutex_t uploadTexturesMutex;

extern uint32 assetThreadCount;
extern pthread_mutex_t assetThreadsMutex;
extern pthread_cond_t assetThreadsCondition;

extern bool assetsChanged;

internal List freeModelsQueue;
internal pthread_mutex_t freeModelsMutex;

internal List freeTexturesQueue;
internal pthread_mutex_t freeTexturesMutex;

internal pthread_t assetManagerThread;

internal bool exitAssetManagerThread;
internal pthread_mutex_t exitAssetManagerMutex;

internal bool updateAssetManagerFlag;
internal pthread_mutex_t updateAssetManagerMutex;
internal pthread_cond_t updateAssetManagerCondition;

internal void* updateAssetManager(void *arg);

void initializeAssetManager(real64 *dt) {
	models = createHashMap(
		sizeof(UUID),
		sizeof(Model),
		MODELS_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&modelsMutex, NULL);

	textures = createHashMap(
		sizeof(UUID),
		sizeof(Texture),
		TEXTURES_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&texturesMutex, NULL);

	materialFolders = createHashMap(
		sizeof(UUID),
		sizeof(List),
		MATERIAL_FOLDERS_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	fonts = createHashMap(
		sizeof(UUID),
		sizeof(Font),
		FONTS_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	images = createHashMap(
		sizeof(UUID),
		sizeof(Image),
		IMAGES_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	particles = createHashMap(
		sizeof(UUID),
		sizeof(Particle),
		PARTICLES_BUCKET_COUNT,
		(ComparisonOp)&strcmp);

	uploadModelsQueue = createHashMap(
		sizeof(UUID),
		sizeof(Model),
		UPLOAD_MODELS_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&uploadModelsMutex, NULL);

	uploadTexturesQueue = createHashMap(
		sizeof(UUID),
		sizeof(Texture),
		UPLOAD_TEXTURES_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&uploadTexturesMutex, NULL);

	freeModelsQueue = createList(sizeof(Model));
	pthread_mutex_init(&freeModelsMutex, NULL);

	freeTexturesQueue = createList(sizeof(Texture));
	pthread_mutex_init(&freeTexturesMutex, NULL);

	assetThreadCount = 0;
	pthread_mutex_init(&assetThreadsMutex, NULL);
	pthread_cond_init(&assetThreadsCondition, NULL);

	assetsChanged = false;

	exitAssetManagerThread = false;
	pthread_mutex_init(&exitAssetManagerMutex, NULL);

	updateAssetManagerFlag = false;
	pthread_mutex_init(&updateAssetManagerMutex, NULL);
	pthread_cond_init(&updateAssetManagerCondition, NULL);

	pthread_create(&assetManagerThread, NULL, &updateAssetManager, dt);
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
		pthread_mutex_lock(&modelsMutex);

		for (HashMapIterator itr = hashMapGetIterator(models);
			!hashMapIteratorAtEnd(itr);)
		{
			Model *model = (Model*)hashMapIteratorGetValue(itr);
			if (model->refCount == 0)
			{
				model->lifetime -= dt;
				if (model->lifetime <= 0.0)
				{
					pthread_mutex_lock(&freeModelsMutex);
					listPushBack(&freeModelsQueue, model);
					pthread_mutex_unlock(&freeModelsMutex);

					UUID modelName = model->name;

					hashMapMoveIterator(&itr);
					hashMapDelete(models, &modelName);

					ASSET_LOG("Model queued to be freed (%s)\n",
							  modelName.string);
					ASSET_LOG("Model Count: %d\n", models->count);
				}
			}
			else
			{
				model->lifetime = config.assetsConfig.minModelLifetime;
				hashMapMoveIterator(&itr);
			}
		}

		pthread_mutex_unlock(&modelsMutex);
		pthread_mutex_lock(&texturesMutex);

		for (HashMapIterator itr = hashMapGetIterator(textures);
			!hashMapIteratorAtEnd(itr);)
		{
			Texture *texture = (Texture*)hashMapIteratorGetValue(itr);
			if (texture->refCount == 0)
			{
				texture->lifetime -= dt;
				if (texture->lifetime <= 0.0)
				{
					pthread_mutex_lock(&freeTexturesMutex);
					listPushBack(&freeTexturesQueue, texture);
					pthread_mutex_unlock(&freeTexturesMutex);

					UUID textureName = texture->name;

					hashMapMoveIterator(&itr);
					hashMapDelete(textures, &textureName);

					ASSET_LOG("Texture queued to be freed (%s)\n",
							  textureName.string);
					ASSET_LOG("Texture Count: %d\n", textures->count);
				}
			}
			else
			{
				texture->lifetime = config.assetsConfig.minTextureLifetime;
				hashMapMoveIterator(&itr);
			}
		}

		pthread_mutex_unlock(&texturesMutex);
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

	pthread_exit(NULL);

	return NULL;
}

void uploadAssets(void)
{
	pthread_mutex_lock(&uploadModelsMutex);

	for (HashMapIterator itr = hashMapGetIterator(uploadModelsQueue);
		 !hashMapIteratorAtEnd(itr);)
	{
		Model *model = hashMapIteratorGetValue(itr);

		pthread_mutex_unlock(&uploadModelsMutex);
		uploadModelToGPU(model);
		pthread_mutex_lock(&uploadModelsMutex);

		pthread_mutex_lock(&modelsMutex);

		UUID modelName = model->name;

		hashMapInsert(models, &modelName, model);
		LOG("Model Count: %d\n", models->count);

		pthread_mutex_unlock(&modelsMutex);

		hashMapMoveIterator(&itr);
		hashMapDelete(uploadModelsQueue, &modelName);
	}

	pthread_mutex_unlock(&uploadModelsMutex);
	pthread_mutex_lock(&uploadTexturesMutex);

	for (HashMapIterator itr = hashMapGetIterator(uploadTexturesQueue);
		 !hashMapIteratorAtEnd(itr);)
	{
		Texture *texture = hashMapIteratorGetValue(itr);

		pthread_mutex_unlock(&uploadTexturesMutex);
		uploadTextureToGPU(texture);
		pthread_mutex_lock(&uploadTexturesMutex);

		pthread_mutex_lock(&texturesMutex);

		UUID textureName = texture->name;

		hashMapInsert(textures, &textureName, texture);
		LOG("Texture Count: %d\n", textures->count);

		pthread_mutex_unlock(&texturesMutex);

		hashMapMoveIterator(&itr);
		hashMapDelete(uploadTexturesQueue, &textureName);
	}

	pthread_mutex_unlock(&uploadTexturesMutex);
}

void freeAssets(void)
{
	pthread_mutex_lock(&freeModelsMutex);

	for (ListIterator listItr = listGetIterator(&freeModelsQueue);
		 !listIteratorAtEnd(listItr);)
	{
		Model *model = LIST_ITERATOR_GET_ELEMENT(Model, listItr);

		pthread_mutex_unlock(&freeModelsMutex);
		freeModelData(model);
		pthread_mutex_lock(&freeModelsMutex);

		listRemove(&freeModelsQueue, &listItr);
	}

	pthread_mutex_unlock(&freeModelsMutex);
	pthread_mutex_lock(&freeTexturesMutex);

	for (ListIterator listItr = listGetIterator(&freeTexturesQueue);
		 !listIteratorAtEnd(listItr);)
	{
		Texture *texture = LIST_ITERATOR_GET_ELEMENT(Texture, listItr);

		pthread_mutex_unlock(&freeTexturesMutex);
		freeTextureData(texture);
		pthread_mutex_lock(&freeTexturesMutex);

		listRemove(&freeTexturesQueue, &listItr);
	}

	pthread_mutex_unlock(&freeTexturesMutex);
}

void shutdownAssetManager(void)
{
	pthread_mutex_lock(&exitAssetManagerMutex);
	exitAssetManagerThread = true;
	pthread_mutex_unlock(&exitAssetManagerMutex);

	setUpdateAssetManagerFlag();

	pthread_join(assetManagerThread, NULL);

	pthread_mutex_destroy(&exitAssetManagerMutex);

	pthread_mutex_destroy(&updateAssetManagerMutex);
	pthread_cond_destroy(&updateAssetManagerCondition);

	pthread_mutex_destroy(&assetThreadsMutex);
	pthread_cond_destroy(&assetThreadsCondition);

	for (HashMapIterator itr = hashMapGetIterator(uploadModelsQueue);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		Model *model = hashMapIteratorGetValue(itr);
		hashMapInsert(models, &model->name, model);
	}

	freeHashMap(&uploadModelsQueue);
	pthread_mutex_destroy(&uploadModelsMutex);

	for (HashMapIterator itr = hashMapGetIterator(uploadTexturesQueue);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		Texture *texture = hashMapIteratorGetValue(itr);
		hashMapInsert(textures, &texture->name, texture);
	}

	freeHashMap(&uploadTexturesQueue);
	pthread_mutex_destroy(&uploadTexturesMutex);

	for (ListIterator listItr = listGetIterator(&freeModelsQueue);
		 !listIteratorAtEnd(listItr);)
	{
		Model *model = LIST_ITERATOR_GET_ELEMENT(Model, listItr);
		hashMapInsert(models, &model->name, model);
	}

	listClear(&freeModelsQueue);
	pthread_mutex_destroy(&freeModelsMutex);

	for (ListIterator listItr = listGetIterator(&freeTexturesQueue);
		 !listIteratorAtEnd(listItr);)
	{
		Texture *texture = LIST_ITERATOR_GET_ELEMENT(Texture, listItr);
		hashMapInsert(textures, &texture->name, texture);
	}

	listClear(&freeTexturesQueue);
	pthread_mutex_destroy(&freeTexturesMutex);

	for (HashMapIterator itr = hashMapGetIterator(models);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		freeModelData(hashMapIteratorGetValue(itr));
	}

	freeHashMap(&models);
	pthread_mutex_destroy(&modelsMutex);

	for (HashMapIterator itr = hashMapGetIterator(textures);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		freeTextureData(hashMapIteratorGetValue(itr));
	}

	freeHashMap(&textures);
	pthread_mutex_destroy(&texturesMutex);

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

	for (HashMapIterator itr = hashMapGetIterator(fonts);
		 !hashMapIteratorAtEnd(itr);)
	{
		Font *font = (Font*)hashMapIteratorGetValue(itr);
		hashMapMoveIterator(&itr);
		freeFont(font);
	}

	freeHashMap(&fonts);
	freeHashMap(&images);
	freeHashMap(&particles);
}

void activateAssetsChangedFlag(void)
{
	assetsChanged = true;
}