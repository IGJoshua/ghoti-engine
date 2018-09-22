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

internal pthread_t assetManagerThread;

internal bool exitAssetManagerThread;
internal pthread_mutex_t exitAssetManagerMutex;

internal bool updateAssetManagerFlag;
internal pthread_mutex_t updateAssetManagerMutex;
internal pthread_cond_t updateAssetManagerCondition;

internal void* updateAssetManager(void *arg);

internal void addFreeModel(Model *model);
internal void addFreeTexture(Texture *texture);

void initializeAssetManager(real64 *dt) {
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

	freeModelsQueue = createList(sizeof(Model));
	freeTexturesQueue = createList(sizeof(Texture));

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

	exitAssetManagerThread = false;
	pthread_mutex_init(&exitAssetManagerMutex, NULL);

	updateAssetManagerFlag = false;
	pthread_mutex_init(&updateAssetManagerMutex, NULL);
	pthread_cond_init(&updateAssetManagerCondition, NULL);

	pthread_create(&assetManagerThread, NULL, &updateAssetManager, dt);
}

void loadAssetAsync(AssetType type, const char *name, const char *filename)
{
	Asset newAsset = {};

	newAsset.type = type;
	newAsset.name = idFromName(name);

	bool duplicate = false;

	pthread_mutex_lock(&assetsQueueMutex);
	for (ListIterator listItr = listGetIterator(&assetsQueue);
		 !listIteratorAtEnd(listItr);
		 listMoveIterator(&listItr))
	{
		Asset *asset = LIST_ITERATOR_GET_ELEMENT(Asset, listItr);
		if (asset->type == type && !strcmp(asset->name.string, name))
		{
			duplicate = true;
			break;
		}
	}

	pthread_mutex_unlock(&assetsQueueMutex);

	if (duplicate)
	{
		return;
	}

	if (filename)
	{
		newAsset.filename = calloc(1, strlen(filename) + 1);
		strcpy(newAsset.filename, filename);
	}

	pthread_mutex_lock(&assetsQueueMutex);
	listPushBack(&assetsQueue, &newAsset);
	pthread_mutex_unlock(&assetsQueueMutex);
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

						if (model->refCount == 1 && !model->uploaded)
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
						Texture *texture = hashMapGetData(
							textures,
							&asset->name);
						pthread_mutex_unlock(&texturesMutex);

						if (texture->refCount == 1 && !texture->uploaded)
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
			!hashMapIteratorAtEnd(itr);)
		{
			Model *model = (Model*)hashMapIteratorGetValue(itr);
			if (model->refCount == 0)
			{
				model->lifetime -= dt;
				if (model->lifetime <= 0.0)
				{
					addFreeModel(model);

					ASSET_LOG("Model queued to be freed (%s)\n",
							  model->name.string);
					ASSET_LOG("Model Count: %d\n", models->count);

					hashMapMoveIterator(&itr);
					UUID modelName = model->name;
					hashMapDelete(models, &modelName);
				}
			}
			else
			{
				model->lifetime = config.assetsConfig.minimumModelLifetime;
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
					addFreeTexture(texture);

					ASSET_LOG("Texture queued to be freed (%s)\n",
							  texture->name.string);
					ASSET_LOG("Texture Count: %d\n", textures->count);

					hashMapMoveIterator(&itr);
					UUID textureName = texture->name;
					hashMapDelete(textures, &textureName);
				}
			}
			else
			{
				texture->lifetime = config.assetsConfig.minimumTextureLifetime;
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

	for (HashMapIterator itr = hashMapGetIterator(models);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		Model *model = (Model*)hashMapIteratorGetValue(itr);
		addFreeModel(model);
	}

	for (HashMapIterator itr = hashMapGetIterator(textures);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		Texture *texture = (Texture*)hashMapIteratorGetValue(itr);
		addFreeTexture(texture);
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
		addFreeModel(model);
	}

	for (ListIterator listItr = listGetIterator(&uploadTexturesQueue);
		 !listIteratorAtEnd(listItr);
		 listMoveIterator(&listItr))
	{
		Texture *texture = *LIST_ITERATOR_GET_ELEMENT(Texture*, listItr);
		addFreeTexture(texture);
	}

	listClear(&uploadModelsQueue);
	listClear(&uploadTexturesQueue);

	freeAssets();

	listClear(&freeModelsQueue);
	listClear(&freeTexturesQueue);

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

void addFreeModel(Model *model)
{
	bool duplicate = false;

	pthread_mutex_lock(&freeModelsMutex);
	for (ListIterator listItr = listGetIterator(&freeModelsQueue);
		 !listIteratorAtEnd(listItr);
		 listMoveIterator(&listItr))
	{
		Model *freeModel = LIST_ITERATOR_GET_ELEMENT(Model, listItr);
		if (!strcmp(freeModel->name.string, model->name.string))
		{
			duplicate = true;
			break;
		}
	}

	if (!duplicate)
	{
		listPushBack(&freeModelsQueue, model);
	}

	pthread_mutex_unlock(&freeModelsMutex);
}

void addFreeTexture(Texture *texture)
{
	bool duplicate = false;

	pthread_mutex_lock(&freeTexturesMutex);
	for (ListIterator listItr = listGetIterator(&freeTexturesQueue);
		 !listIteratorAtEnd(listItr);
		 listMoveIterator(&listItr))
	{
		Texture *freeTexture = LIST_ITERATOR_GET_ELEMENT(Texture, listItr);
		if (!strcmp(freeTexture->name.string, texture->name.string))
		{
			duplicate = true;
			break;
		}
	}

	if (!duplicate)
	{
		listPushBack(&freeTexturesQueue, texture);
	}

	pthread_mutex_unlock(&freeTexturesMutex);
}