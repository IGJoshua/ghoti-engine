#include "asset_management/asset_manager.h"
#include "asset_management/asset_manager_types.h"
#include "asset_management/model.h"
#include "asset_management/texture.h"
#include "asset_management/image.h"
#include "asset_management/font.h"
#include "asset_management/audio.h"

#include "components/component_types.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include <string.h>
#include <pthread.h>

extern Config config;

// Resource HashMaps

extern HashMap models;
extern pthread_mutex_t modelsMutex;

extern HashMap textures;
extern pthread_mutex_t texturesMutex;

extern HashMap materialFolders;
extern pthread_mutex_t materialFoldersMutex;

extern HashMap fonts;

extern HashMap images;
extern pthread_mutex_t imagesMutex;

extern HashMap audioFiles;
extern HashMap particles;

// Resource Loading HashMaps

extern HashMap loadingModels;
extern pthread_mutex_t loadingModelsMutex;

extern HashMap loadingTextures;
extern pthread_mutex_t loadingTexturesMutex;

extern HashMap loadingImages;
extern pthread_mutex_t loadingImagesMutex;

// Resource Uploading HashMaps

extern HashMap uploadModelsQueue;
extern pthread_mutex_t uploadModelsMutex;

extern HashMap uploadTexturesQueue;
extern pthread_mutex_t uploadTexturesMutex;

extern HashMap uploadImagesQueue;
extern pthread_mutex_t uploadImagesMutex;

// Resource Freeing Lists

internal List freeModelsQueue;
internal pthread_mutex_t freeModelsMutex;

internal List freeTexturesQueue;
internal pthread_mutex_t freeTexturesMutex;

internal List freeImagesQueue;
internal pthread_mutex_t freeImagesMutex;

// Asset Management Globals

extern uint32 assetThreadCount;
extern pthread_mutex_t assetThreadsMutex;
extern pthread_cond_t assetThreadsCondition;

extern pthread_mutex_t devilMutex;

// Asset Management Locals

internal pthread_t assetManagerThread;

internal bool exitAssetManagerThread;
internal pthread_mutex_t exitAssetManagerMutex;

internal bool updateAssetManagerFlag;
internal pthread_mutex_t updateAssetManagerMutex;
internal pthread_cond_t updateAssetManagerCondition;

internal void* updateAssetManager(void *arg);

void initializeAssetManager(real64 *dt) {
	// Resource HashMaps

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
	pthread_mutex_init(&materialFoldersMutex, NULL);

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
	pthread_mutex_init(&imagesMutex, NULL);

	audioFiles = createHashMap(
		sizeof(UUID),
		sizeof(AudioFile),
		AUDIO_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	particles = createHashMap(
		sizeof(UUID),
		sizeof(Particle),
		PARTICLES_BUCKET_COUNT,
		(ComparisonOp)&strcmp);

	// Resource Uploading HashMaps

	loadingModels = createHashMap(
		sizeof(UUID),
		sizeof(bool),
		LOADING_MODELS_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&loadingModelsMutex, NULL);

	loadingTextures = createHashMap(
		sizeof(UUID),
		sizeof(Texture),
		LOADING_TEXTURES_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&loadingTexturesMutex, NULL);

	loadingImages = createHashMap(
		sizeof(UUID),
		sizeof(Image),
		LOADING_IMAGES_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&loadingImagesMutex, NULL);

	// Resource Uploading HashMaps

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

	uploadImagesQueue = createHashMap(
		sizeof(UUID),
		sizeof(Image),
		UPLOAD_IMAGES_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&uploadImagesMutex, NULL);

	// Resource Freeing Lists

	freeModelsQueue = createList(sizeof(Model));
	pthread_mutex_init(&freeModelsMutex, NULL);

	freeTexturesQueue = createList(sizeof(Texture));
	pthread_mutex_init(&freeTexturesMutex, NULL);

	freeImagesQueue = createList(sizeof(Image));
	pthread_mutex_init(&freeImagesMutex, NULL);

	// Asset Management Globals

	assetThreadCount = 0;
	pthread_mutex_init(&assetThreadsMutex, NULL);
	pthread_cond_init(&assetThreadsCondition, NULL);

	pthread_mutex_init(&devilMutex, NULL);

	// Asset Management Locals

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

		// Free Models

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

					ASSET_LOG(
						MODEL,
						modelName.string,
						"Model queued to be freed (%s)\n",
						modelName.string);
					ASSET_LOG(
						MODEL,
						modelName.string,
						"Model Count: %d\n",
						models->count);
					ASSET_LOG_COMMIT(MODEL, modelName.string);
				}

				hashMapMoveIterator(&itr);
			}
			else
			{
				model->lifetime = config.assetsConfig.minModelLifetime;
				hashMapMoveIterator(&itr);
			}
		}

		pthread_mutex_unlock(&modelsMutex);

		// Free Textures

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

					ASSET_LOG(
						TEXTURE,
						textureName.string,
						"Texture queued to be freed (%s)\n",
						textureName.string);
					ASSET_LOG(
						TEXTURE,
						textureName.string,
						"Texture Count: %d\n",
						textures->count);
					ASSET_LOG_COMMIT(TEXTURE, textureName.string);
				}

				hashMapMoveIterator(&itr);
			}
			else
			{
				texture->lifetime = config.assetsConfig.minTextureLifetime;
				hashMapMoveIterator(&itr);
			}
		}

		pthread_mutex_unlock(&texturesMutex);

		// Free Images

		pthread_mutex_lock(&imagesMutex);

		for (HashMapIterator itr = hashMapGetIterator(images);
			!hashMapIteratorAtEnd(itr);)
		{
			Image *image = (Image*)hashMapIteratorGetValue(itr);
			if (image->refCount == 0)
			{
				image->lifetime -= dt;
				if (image->lifetime <= 0.0)
				{
					pthread_mutex_lock(&freeImagesMutex);
					listPushBack(&freeImagesQueue, image);
					pthread_mutex_unlock(&freeImagesMutex);

					UUID imageName = image->name;

					hashMapMoveIterator(&itr);
					hashMapDelete(images, &imageName);

					ASSET_LOG(
						IMAGE,
						imageName.string,
						"Image queued to be freed (%s)\n",
						imageName.string);
					ASSET_LOG(
						IMAGE,
						imageName.string,
						"Image Count: %d\n",
						images->count);
					ASSET_LOG_COMMIT(IMAGE, imageName.string);
				}

				hashMapMoveIterator(&itr);
			}
			else
			{
				image->lifetime = config.assetsConfig.minImageLifetime;
				hashMapMoveIterator(&itr);
			}
		}

		pthread_mutex_unlock(&imagesMutex);
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
	// Upload Models

	pthread_mutex_lock(&uploadModelsMutex);

	for (HashMapIterator itr = hashMapGetIterator(uploadModelsQueue);
		 !hashMapIteratorAtEnd(itr);)
	{
		Model *model = hashMapIteratorGetValue(itr);

		pthread_mutex_unlock(&uploadModelsMutex);
		uploadModelToGPU(model);
		pthread_mutex_lock(&uploadModelsMutex);

		UUID modelName = model->name;

		pthread_mutex_lock(&modelsMutex);

		hashMapInsert(models, &modelName, model);
		LOG("Model Count: %d\n", models->count);

		pthread_mutex_unlock(&modelsMutex);

		hashMapMoveIterator(&itr);
		hashMapDelete(uploadModelsQueue, &modelName);
	}

	pthread_mutex_unlock(&uploadModelsMutex);

	// Upload Textures

	pthread_mutex_lock(&uploadTexturesMutex);

	for (HashMapIterator itr = hashMapGetIterator(uploadTexturesQueue);
		 !hashMapIteratorAtEnd(itr);)
	{
		Texture *texture = hashMapIteratorGetValue(itr);

		pthread_mutex_unlock(&uploadTexturesMutex);
		uploadTextureToGPU(texture);
		pthread_mutex_lock(&uploadTexturesMutex);

		UUID textureName = texture->name;

		pthread_mutex_lock(&texturesMutex);

		hashMapInsert(textures, &textureName, texture);
		LOG("Texture Count: %d\n", textures->count);

		pthread_mutex_unlock(&texturesMutex);

		hashMapMoveIterator(&itr);
		hashMapDelete(uploadTexturesQueue, &textureName);
	}

	pthread_mutex_unlock(&uploadTexturesMutex);

	// Upload Images

	pthread_mutex_lock(&uploadImagesMutex);

	for (HashMapIterator itr = hashMapGetIterator(uploadImagesQueue);
		 !hashMapIteratorAtEnd(itr);)
	{
		Image *image = hashMapIteratorGetValue(itr);

		pthread_mutex_unlock(&uploadImagesMutex);
		uploadImageToGPU(image);
		pthread_mutex_lock(&uploadImagesMutex);

		UUID imageName = image->name;

		pthread_mutex_lock(&imagesMutex);

		hashMapInsert(images, &imageName, image);
		LOG("Image Count: %d\n", images->count);

		pthread_mutex_unlock(&imagesMutex);

		hashMapMoveIterator(&itr);
		hashMapDelete(uploadImagesQueue, &imageName);
	}

	pthread_mutex_unlock(&uploadImagesMutex);
}

void freeAssets(void)
{
	// Free Models

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

	// Free Textures

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

	// Free Images

	pthread_mutex_lock(&freeImagesMutex);

	for (ListIterator listItr = listGetIterator(&freeImagesQueue);
		 !listIteratorAtEnd(listItr);)
	{
		Image *image = LIST_ITERATOR_GET_ELEMENT(Image, listItr);

		pthread_mutex_unlock(&freeImagesMutex);
		freeImageData(image);
		pthread_mutex_lock(&freeImagesMutex);

		listRemove(&freeImagesQueue, &listItr);
	}

	pthread_mutex_unlock(&freeImagesMutex);
}

void shutdownAssetManager(void)
{
	// Asset Management Locals

	pthread_mutex_lock(&exitAssetManagerMutex);
	exitAssetManagerThread = true;
	pthread_mutex_unlock(&exitAssetManagerMutex);

	setUpdateAssetManagerFlag();

	pthread_join(assetManagerThread, NULL);

	pthread_mutex_destroy(&exitAssetManagerMutex);

	pthread_mutex_destroy(&updateAssetManagerMutex);
	pthread_cond_destroy(&updateAssetManagerCondition);

	// Asset Management Globals

	pthread_mutex_lock(&assetThreadsMutex);

	while (assetThreadCount > 0)
	{
		pthread_cond_wait(&assetThreadsCondition, &assetThreadsMutex);
	}

	pthread_mutex_unlock(&assetThreadsMutex);

	pthread_mutex_destroy(&assetThreadsMutex);
	pthread_cond_destroy(&assetThreadsCondition);

	pthread_mutex_destroy(&devilMutex);

	// Resource Freeing Lists

	for (ListIterator listItr = listGetIterator(&freeModelsQueue);
		 !listIteratorAtEnd(listItr);
		 listMoveIterator(&listItr))
	{
		Model *model = LIST_ITERATOR_GET_ELEMENT(Model, listItr);
		hashMapInsert(models, &model->name, model);
	}

	listClear(&freeModelsQueue);
	pthread_mutex_destroy(&freeModelsMutex);

	for (ListIterator listItr = listGetIterator(&freeTexturesQueue);
		 !listIteratorAtEnd(listItr);
		 listMoveIterator(&listItr))
	{
		Texture *texture = LIST_ITERATOR_GET_ELEMENT(Texture, listItr);
		hashMapInsert(textures, &texture->name, texture);
	}

	listClear(&freeTexturesQueue);
	pthread_mutex_destroy(&freeTexturesMutex);

	for (ListIterator listItr = listGetIterator(&freeImagesQueue);
		 !listIteratorAtEnd(listItr);
		 listMoveIterator(&listItr))
	{
		Image *image = LIST_ITERATOR_GET_ELEMENT(Image, listItr);
		hashMapInsert(images, &image->name, image);
	}

	listClear(&freeImagesQueue);
	pthread_mutex_destroy(&freeImagesMutex);

	// Resource Uploading HashMaps

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

	for (HashMapIterator itr = hashMapGetIterator(uploadImagesQueue);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		Image *image = hashMapIteratorGetValue(itr);
		hashMapInsert(images, &image->name, image);
	}

	freeHashMap(&uploadImagesQueue);
	pthread_mutex_destroy(&uploadImagesMutex);

	// Resource Loading HashMaps

	freeHashMap(&loadingModels);
	pthread_mutex_destroy(&loadingModelsMutex);

	freeHashMap(&loadingTextures);
	pthread_mutex_destroy(&loadingTexturesMutex);

	freeHashMap(&loadingImages);
	pthread_mutex_destroy(&loadingImagesMutex);

	// Resource HashMaps

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
	pthread_mutex_destroy(&materialFoldersMutex);

	for (HashMapIterator itr = hashMapGetIterator(fonts);
		 !hashMapIteratorAtEnd(itr);)
	{
		Font *font = (Font*)hashMapIteratorGetValue(itr);
		hashMapMoveIterator(&itr);
		freeFont(font);
	}

	freeHashMap(&fonts);

	for (HashMapIterator itr = hashMapGetIterator(images);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		freeImageData(hashMapIteratorGetValue(itr));
	}

	freeHashMap(&images);

	for (HashMapIterator itr = hashMapGetIterator(audioFiles);
		!hashMapIteratorAtEnd(itr);)
	{
		AudioFile *audio = (AudioFile*)hashMapIteratorGetValue(itr);
		hashMapMoveIterator(&itr);
		freeAudio(audio);
	}

	freeHashMap(&audioFiles);
	freeHashMap(&particles);
}