#include "asset_management/asset_manager.h"
#include "asset_management/model.h"
#include "asset_management/texture.h"
#include "asset_management/font.h"

#include "components/component_types.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/scene.h"

#include <string.h>
#include <pthread.h>

typedef struct asset_t
{
	AssetType type;
	UUID name;
	char *filename;
} Asset;

extern Config config;

extern HashMap models;
extern HashMap textures;
extern HashMap materialFolders;
extern HashMap fonts;
extern HashMap images;
extern HashMap particles;

extern pthread_mutex_t modelsMutex;
extern pthread_mutex_t texturesMutex;

internal List assetsQueue;

internal List uploadModelsQueue;
internal List uploadTexturesQueue;

internal List freeModelsQueue;
internal List freeTexturesQueue;

internal pthread_mutex_t assetsQueueMutex;

internal pthread_mutex_t uploadModelsMutex;
internal pthread_mutex_t uploadTexturesMutex;

internal pthread_mutex_t freeModelsMutex;
internal pthread_mutex_t freeTexturesMutex;

extern bool assetsChanged;
extern bool asyncAssetLoading;

extern pthread_mutex_t asyncAssetLoadingMutex;

void initializeAssetManager(void) {
	models = createHashMap(
		sizeof(UUID),
		sizeof(Model),
		MODELS_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	textures = createHashMap(
		sizeof(UUID),
		sizeof(Texture),
		TEXTURES_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
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

	assetsQueue = createList(sizeof(Asset));

	uploadModelsQueue = createList(sizeof(Model*));
	uploadTexturesQueue = createList(sizeof(Texture*));

	freeModelsQueue = createList(sizeof(Model*));
	freeTexturesQueue = createList(sizeof(Texture*));

	assetsChanged = false;
	asyncAssetLoading = true;

	pthread_mutex_init(&modelsMutex, NULL);
	pthread_mutex_init(&texturesMutex, NULL);
	pthread_mutex_init(&assetsQueueMutex, NULL);
	pthread_mutex_init(&uploadModelsMutex, NULL);
	pthread_mutex_init(&uploadTexturesMutex, NULL);
	pthread_mutex_init(&freeModelsMutex, NULL);
	pthread_mutex_init(&freeTexturesMutex, NULL);
	pthread_mutex_init(&asyncAssetLoadingMutex, NULL);
}

void loadAssetAsync(AssetType type, const char *name, const char *filename)
{
	Asset asset = {};

	asset.type = type;
	asset.name = idFromName(name);

	if (filename)
	{
		asset.filename = calloc(1, strlen(filename) + 1);
		strcpy(asset.filename, filename);
	}

	pthread_mutex_lock(&assetsQueueMutex);
	listPushBack(&assetsQueue, &asset);
	pthread_mutex_unlock(&assetsQueueMutex);
}

void updateAssetManager(real64 dt)
{
	pthread_mutex_lock(&assetsQueueMutex);
	for (ListIterator listItr = listGetIterator(&assetsQueue);
		 !listIteratorAtEnd(listItr);)
	{
		int32 error = 0;

		Asset *asset = LIST_ITERATOR_GET_ELEMENT(Asset, listItr);
		switch (asset->type)
		{
			case ASSET_TYPE_MODEL:
				pthread_mutex_unlock(&assetsQueueMutex);
				error = loadModel(asset->name.string);
				pthread_mutex_lock(&assetsQueueMutex);

				if (error != -1)
				{
					pthread_mutex_lock(&modelsMutex);
					Model *model = hashMapGetData(models, &asset->name);
					pthread_mutex_unlock(&modelsMutex);

					if (model->refCount == 1)
					{
						pthread_mutex_lock(&uploadModelsMutex);
						listPushBack(&uploadModelsQueue, &model);
						pthread_mutex_unlock(&uploadModelsMutex);
					}
				}

				break;
			case ASSET_TYPE_TEXTURE:
				pthread_mutex_unlock(&assetsQueueMutex);
				error = loadTexture(asset->filename, asset->name.string);
				pthread_mutex_lock(&assetsQueueMutex);

				if (error != -1)
				{
					pthread_mutex_lock(&texturesMutex);
					Texture *texture = hashMapGetData(textures, &asset->name);
					pthread_mutex_unlock(&texturesMutex);

					if (texture->refCount == 1)
					{
						pthread_mutex_lock(&uploadTexturesMutex);
						listPushBack(&uploadTexturesQueue, &texture);
						pthread_mutex_unlock(&uploadTexturesMutex);
					}
				}

				break;
			default:
				break;
		}

		free(asset->filename);

		listRemove(&assetsQueue, &listItr);
	}

	pthread_mutex_unlock(&assetsQueueMutex);

	pthread_mutex_lock(&modelsMutex);
	for (HashMapIterator itr = hashMapGetIterator(models);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		Model *model = (Model*)hashMapIteratorGetValue(itr);
		if (model->refCount == 0)
		{
			model->lifetime -= dt;
			if (model->lifetime <= 0.0)
			{
				pthread_mutex_lock(&freeModelsMutex);
				listPushBack(&freeModelsQueue, &model);
				pthread_mutex_unlock(&freeModelsMutex);
			}
		}
		else
		{
			model->lifetime = config.assetsConfig.minimumModelLifetime;
		}
	}

	pthread_mutex_unlock(&modelsMutex);

	pthread_mutex_lock(&texturesMutex);
	for (HashMapIterator itr = hashMapGetIterator(textures);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		Texture *texture = (Texture*)hashMapIteratorGetValue(itr);
		if (texture->refCount == 0)
		{
			texture->lifetime -= dt;
			if (texture->lifetime <= 0.0)
			{
				pthread_mutex_lock(&freeTexturesMutex);
				listPushBack(&freeTexturesQueue, &texture);
				pthread_mutex_unlock(&freeTexturesMutex);
			}
		}
		else
		{
			texture->lifetime = config.assetsConfig.minimumTextureLifetime;
		}
	}

	pthread_mutex_unlock(&texturesMutex);
}

void uploadAssets(void)
{
	pthread_mutex_lock(&uploadModelsMutex);
	for (ListIterator listItr = listGetIterator(&uploadModelsQueue);
		 !listIteratorAtEnd(listItr);)
	{
		Model *model = *LIST_ITERATOR_GET_ELEMENT(Model*, listItr);

		pthread_mutex_unlock(&uploadModelsMutex);
		uploadModelToGPU(model);
		pthread_mutex_lock(&uploadModelsMutex);

		listRemove(&uploadModelsQueue, &listItr);
	}

	pthread_mutex_unlock(&uploadModelsMutex);

	pthread_mutex_lock(&uploadTexturesMutex);
	for (ListIterator listItr = listGetIterator(&uploadTexturesQueue);
		 !listIteratorAtEnd(listItr);)
	{
		Texture *texture = *LIST_ITERATOR_GET_ELEMENT(Texture*, listItr);

		pthread_mutex_unlock(&uploadTexturesMutex);
		uploadTextureToGPU(texture);
		pthread_mutex_lock(&uploadTexturesMutex);

		listRemove(&uploadTexturesQueue, &listItr);
	}

	pthread_mutex_unlock(&uploadTexturesMutex);
}

void freeAssets(void)
{
	pthread_mutex_lock(&freeModelsMutex);
	for (ListIterator listItr = listGetIterator(&freeModelsQueue);
		 !listIteratorAtEnd(listItr);)
	{
		Model *model = *LIST_ITERATOR_GET_ELEMENT(Model*, listItr);

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
		Texture *texture = *LIST_ITERATOR_GET_ELEMENT(Texture*, listItr);

		pthread_mutex_unlock(&freeTexturesMutex);
		freeTextureData(texture);
		pthread_mutex_lock(&freeTexturesMutex);

		listRemove(&freeTexturesQueue, &listItr);
	}

	pthread_mutex_unlock(&freeTexturesMutex);
}

void shutdownAssetManager(void)
{
	for (HashMapIterator itr = hashMapGetIterator(models);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		Model *model = (Model*)hashMapIteratorGetValue(itr);
		listPushBack(&freeModelsQueue, &model);
	}

	for (HashMapIterator itr = hashMapGetIterator(textures);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		Texture *texture = (Texture*)hashMapIteratorGetValue(itr);
		listPushBack(&freeTexturesQueue, &texture);
	}

	for (ListIterator listItr = listGetIterator(&assetsQueue);
		 !listIteratorAtEnd(listItr);)
	{
		Asset *asset = LIST_ITERATOR_GET_ELEMENT(Asset, listItr);
		free(asset->filename);
	}

	listClear(&assetsQueue);

	for (ListIterator listItr = listGetIterator(&uploadModelsQueue);
		 !listIteratorAtEnd(listItr);
		 listMoveIterator(&listItr))
	{
		Model *model = *LIST_ITERATOR_GET_ELEMENT(Model*, listItr);
		listPushBack(&freeModelsQueue, &model);
	}

	for (ListIterator listItr = listGetIterator(&uploadTexturesQueue);
		 !listIteratorAtEnd(listItr);
		 listMoveIterator(&listItr))
	{
		Texture *texture = *LIST_ITERATOR_GET_ELEMENT(Texture*, listItr);
		listPushBack(&freeTexturesQueue, &texture);
	}

	listClear(&uploadModelsQueue);
	listClear(&uploadTexturesQueue);

	freeAssets();

	listClear(&freeModelsQueue);
	listClear(&freeModelsQueue);

	freeHashMap(&models);
	freeHashMap(&textures);

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

	pthread_mutex_destroy(&modelsMutex);
	pthread_mutex_destroy(&texturesMutex);
	pthread_mutex_destroy(&assetsQueueMutex);
	pthread_mutex_destroy(&uploadModelsMutex);
	pthread_mutex_destroy(&uploadTexturesMutex);
	pthread_mutex_destroy(&freeModelsMutex);
	pthread_mutex_destroy(&freeTexturesMutex);
	pthread_mutex_destroy(&asyncAssetLoadingMutex);
}

void activateAssetsChangedFlag(void)
{
	assetsChanged = true;
}

void setAsyncAssetLoading(bool async)
{
	pthread_mutex_lock(&asyncAssetLoadingMutex);
	asyncAssetLoading = async;
	pthread_mutex_unlock(&asyncAssetLoadingMutex);
}