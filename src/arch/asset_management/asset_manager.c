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

internal List assetsQueue;

internal List uploadModelsQueue;
internal List uploadTexturesQueue;

internal List freeModelsQueue;
internal List freeTexturesQueue;

extern bool assetsChanged;
extern bool asyncAssetLoading;

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
}

void updateAssetManager(real64 dt)
{
	// Lock mutex
	for (ListIterator listItr = listGetIterator(&assetsQueue);
		 !listIteratorAtEnd(listItr);)
	{
		int32 error = 0;

		Asset *asset = LIST_ITERATOR_GET_ELEMENT(Asset, listItr);
		switch (asset->type)
		{
			case ASSET_TYPE_MODEL:
				error = loadModel(asset->name.string);

				if (error != -1)
				{
					// Lock mutex
					Model *model = hashMapGetData(models, &asset->name);
					// Unlock mutex

					if (model->refCount == 1)
					{
						// Lock mutex
						listPushBack(&uploadModelsQueue, &model);
						// Unlock mutex
					}
				}

				break;
			case ASSET_TYPE_TEXTURE:
				error = loadTexture(asset->filename, asset->name.string);

				if (error != -1)
				{
					// Lock mutex
					Texture *texture = hashMapGetData(textures, &asset->name);
					// Unlock mutex

					if (texture->refCount == 1)
					{
						// Lock mutex
						listPushBack(&uploadTexturesQueue, &texture);
						// Unlock mutex
					}
				}

				break;
			default:
				break;
		}

		free(asset->filename);

		listRemove(&assetsQueue, &listItr);
	}
	// Unlock mutex

	// Lock mutex
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
				// Lock mutex
				listPushBack(&freeModelsQueue, &model);
				// Unlock mutex
			}
		}
		else
		{
			model->lifetime = config.assetsConfig.minimumModelLifetime;
		}
	}

	// Unlock mutex

	// Lock mutex
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
				// Lock mutex
				listPushBack(&freeTexturesQueue, &texture);
				// Unlock mutex
			}
		}
		else
		{
			texture->lifetime = config.assetsConfig.minimumTextureLifetime;
		}
	}

	// Unlock mutex
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
}

void uploadAssets(void)
{
	// Lock mutex
	for (ListIterator listItr = listGetIterator(&uploadModelsQueue);
		 !listIteratorAtEnd(listItr);)
	{
		Model *model = *LIST_ITERATOR_GET_ELEMENT(Model*, listItr);

		// Unlock mutex
		uploadModelToGPU(model);
		// Lock mutex

		listRemove(&uploadModelsQueue, &listItr);
	}

	// Unlock mutex

	// Lock mutex
	for (ListIterator listItr = listGetIterator(&uploadTexturesQueue);
		 !listIteratorAtEnd(listItr);)
	{
		Texture *texture = *LIST_ITERATOR_GET_ELEMENT(Texture*, listItr);

		// Unlock mutex
		uploadTextureToGPU(texture);
		// Lock mutex

		listRemove(&uploadTexturesQueue, &listItr);
	}

	// Unlock mutex
}

void freeAssets(void)
{
	// Lock mutex
	for (ListIterator listItr = listGetIterator(&freeModelsQueue);
		 !listIteratorAtEnd(listItr);)
	{
		Model *model = *LIST_ITERATOR_GET_ELEMENT(Model*, listItr);

		// Unlock mutex
		freeModelData(model);
		// Lock mutex

		listRemove(&freeModelsQueue, &listItr);
	}

	// Unlock mutex

	// Lock mutex
	for (ListIterator listItr = listGetIterator(&freeTexturesQueue);
		 !listIteratorAtEnd(listItr);)
	{
		Texture *texture = *LIST_ITERATOR_GET_ELEMENT(Texture*, listItr);

		// Unlock mutex
		freeTextureData(texture);
		// Lock mutex

		listRemove(&freeTexturesQueue, &listItr);
	}

	// Unlock mutex
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

	// Lock mutex
	listPushBack(&assetsQueue, &asset);
	// Unlock mutex
}

void activateAssetsChangedFlag(void)
{
	assetsChanged = true;
}

void setAsyncAssetLoading(bool async)
{
	// Lock mutex
	asyncAssetLoading = async;
	// Unlock mutex
}