#include "asset_management/asset_manager.h"
#include "asset_management/asset_manager_types.h"
#include "asset_management/model.h"
#include "asset_management/texture.h"
#include "asset_management/font.h"
#include "asset_management/image.h"
#include "asset_management/audio.h"
#include "asset_management/particle.h"

#include "components/component_types.h"
#include "components/audio_source.h"

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
extern pthread_mutex_t fontsMutex;

extern HashMap images;
extern pthread_mutex_t imagesMutex;

extern HashMap audioFiles;
extern pthread_mutex_t audioMutex;

extern HashMap particles;
extern pthread_mutex_t particlesMutex;

// Resource Loading HashMaps

extern HashMap loadingModels;
extern pthread_mutex_t loadingModelsMutex;

extern HashMap loadingTextures;
extern pthread_mutex_t loadingTexturesMutex;

extern HashMap loadingFonts;
extern pthread_mutex_t loadingFontsMutex;

extern HashMap loadingImages;
extern pthread_mutex_t loadingImagesMutex;

extern HashMap loadingAudio;
extern pthread_mutex_t loadingAudioMutex;

extern HashMap loadingParticles;
extern pthread_mutex_t loadingParticlesMutex;

// Resource Uploading HashMaps

extern HashMap uploadModelsQueue;
extern pthread_mutex_t uploadModelsMutex;

extern HashMap uploadTexturesQueue;
extern pthread_mutex_t uploadTexturesMutex;

extern HashMap uploadFontsQueue;
extern pthread_mutex_t uploadFontsMutex;

extern HashMap uploadImagesQueue;
extern pthread_mutex_t uploadImagesMutex;

extern HashMap uploadAudioQueue;
extern pthread_mutex_t uploadAudioMutex;

extern HashMap uploadParticlesQueue;
extern pthread_mutex_t uploadParticlesMutex;

// Resource Freeing Lists

internal List freeModelsQueue;
internal pthread_mutex_t freeModelsMutex;

internal List freeTexturesQueue;
internal pthread_mutex_t freeTexturesMutex;

internal List freeFontsQueue;
internal pthread_mutex_t freeFontsMutex;

internal List freeImagesQueue;
internal pthread_mutex_t freeImagesMutex;

internal List freeAudioQueue;
internal pthread_mutex_t freeAudioMutex;

internal List freeParticleQueue;
internal pthread_mutex_t freeParticleMutex;

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
	pthread_mutex_init(&fontsMutex, NULL);

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
	pthread_mutex_init(&audioMutex, NULL);

	particles = createHashMap(
		sizeof(UUID),
		sizeof(Particle),
		PARTICLES_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&particlesMutex, NULL);

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

	loadingFonts = createHashMap(
		sizeof(UUID),
		sizeof(Font),
		LOADING_FONTS_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&loadingFontsMutex, NULL);

	loadingImages = createHashMap(
		sizeof(UUID),
		sizeof(Image),
		LOADING_IMAGES_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&loadingImagesMutex, NULL);

	loadingAudio = createHashMap(
		sizeof(UUID),
		sizeof(AudioFile),
		LOADING_AUDIO_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&loadingAudioMutex, NULL);

	loadingParticles = createHashMap(
		sizeof(UUID),
		sizeof(Particle),
		LOADING_PARTICLES_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&loadingParticlesMutex, NULL);

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

	uploadFontsQueue = createHashMap(
		sizeof(UUID),
		sizeof(Font),
		UPLOAD_FONTS_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&uploadFontsMutex, NULL);

	uploadImagesQueue = createHashMap(
		sizeof(UUID),
		sizeof(Image),
		UPLOAD_IMAGES_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&uploadImagesMutex, NULL);

	uploadAudioQueue = createHashMap(
		sizeof(UUID),
		sizeof(AudioFile),
		UPLOAD_AUDIO_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&uploadAudioMutex, NULL);

	uploadParticlesQueue = createHashMap(
		sizeof(UUID),
		sizeof(Particle),
		UPLOAD_PARTICLES_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&uploadParticlesMutex, NULL);

	// Resource Freeing Lists

	freeModelsQueue = createList(sizeof(Model));
	pthread_mutex_init(&freeModelsMutex, NULL);

	freeTexturesQueue = createList(sizeof(Texture));
	pthread_mutex_init(&freeTexturesMutex, NULL);

	freeFontsQueue = createList(sizeof(Font));
	pthread_mutex_init(&freeFontsMutex, NULL);

	freeImagesQueue = createList(sizeof(Image));
	pthread_mutex_init(&freeImagesMutex, NULL);

	freeAudioQueue = createList(sizeof(AudioFile));
	pthread_mutex_init(&freeAudioMutex, NULL);

	freeParticleQueue = createList(sizeof(Particle));
	pthread_mutex_init(&freeParticleMutex, NULL);

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
				else
				{
					hashMapMoveIterator(&itr);
				}
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
				else
				{
					hashMapMoveIterator(&itr);
				}
			}
			else
			{
				texture->lifetime = config.assetsConfig.minTextureLifetime;
				hashMapMoveIterator(&itr);
			}
		}

		pthread_mutex_unlock(&texturesMutex);

		// Free Fonts

		pthread_mutex_lock(&fontsMutex);

		for (HashMapIterator itr = hashMapGetIterator(fonts);
			 !hashMapIteratorAtEnd(itr);)
		{
			Font *font = (Font*)hashMapIteratorGetValue(itr);
			font->lifetime -= dt;

			if (font->lifetime <= 0.0)
			{
				pthread_mutex_lock(&freeFontsMutex);
				listPushBack(&freeFontsQueue, font);
				pthread_mutex_unlock(&freeFontsMutex);

				UUID fontName = font->name;

				hashMapMoveIterator(&itr);
				hashMapDelete(fonts, &fontName);

				ASSET_LOG(
					FONT,
					fontName.string,
					"Font queued to be freed (%s)\n",
					fontName.string);
				ASSET_LOG(
					FONT,
					fontName.string,
					"Font Count: %d\n",
					fonts->count);
				ASSET_LOG_COMMIT(FONT, fontName.string);
			}
			else
			{
				hashMapMoveIterator(&itr);
			}
		}

		pthread_mutex_unlock(&fontsMutex);

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
				else
				{
					hashMapMoveIterator(&itr);
				}
			}
			else
			{
				image->lifetime = config.assetsConfig.minImageLifetime;
				hashMapMoveIterator(&itr);
			}
		}

		pthread_mutex_unlock(&imagesMutex);

		// Free Audio

		pthread_mutex_lock(&audioMutex);

		for (HashMapIterator itr = hashMapGetIterator(audioFiles);
			 !hashMapIteratorAtEnd(itr);)
		{
			AudioFile *audio = (AudioFile*)hashMapIteratorGetValue(itr);
			audio->lifetime -= dt;

			if (audio->lifetime <= 0.0)
			{
				pthread_mutex_lock(&freeAudioMutex);
				listPushBack(&freeAudioQueue, audio);
				pthread_mutex_unlock(&freeAudioMutex);

				UUID audioName = audio->name;

				hashMapMoveIterator(&itr);
				hashMapDelete(audioFiles, &audioName);

				ASSET_LOG(
					AUDIO,
					audioName.string,
					"Audio queued to be freed (%s)\n",
					audioName.string);
				ASSET_LOG(
					AUDIO,
					audioName.string,
					"Audio Count: %d\n",
					audioFiles->count);
				ASSET_LOG_COMMIT(AUDIO, audioName.string);
			}
			else
			{
				hashMapMoveIterator(&itr);
			}
		}

		pthread_mutex_unlock(&audioMutex);

		// Free Particles

		pthread_mutex_lock(&particlesMutex);

		for (HashMapIterator itr = hashMapGetIterator(particles);
			 !hashMapIteratorAtEnd(itr);)
		{
			Particle *particle = (Particle*)hashMapIteratorGetValue(itr);
			particle->lifetime -= dt;

			if (particle->lifetime <= 0.0)
			{
				pthread_mutex_lock(&freeParticleMutex);
				listPushBack(&freeParticleQueue, particle);
				pthread_mutex_unlock(&freeParticleMutex);

				UUID particleName = particle->name;

				hashMapMoveIterator(&itr);
				hashMapDelete(particles, &particleName);

				ASSET_LOG(
					PARTICLE,
					particleName.string,
					"Particle queued to be freed (%s)\n",
					particleName.string);
				ASSET_LOG(
					PARTICLE,
					particleName.string,
					"Particle Count: %d\n",
					particles->count);
				ASSET_LOG_COMMIT(PARTICLE, particleName.string);
			}
			else
			{
				hashMapMoveIterator(&itr);
			}
		}

		pthread_mutex_unlock(&particlesMutex);
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

	// Upload Fonts

	pthread_mutex_lock(&uploadFontsMutex);

	for (HashMapIterator itr = hashMapGetIterator(uploadFontsQueue);
		 !hashMapIteratorAtEnd(itr);)
	{
		Font *font = hashMapIteratorGetValue(itr);

		pthread_mutex_unlock(&uploadFontsMutex);
		uploadFontToGPU(font);
		pthread_mutex_lock(&uploadFontsMutex);

		UUID fontName = font->name;

		pthread_mutex_lock(&fontsMutex);

		hashMapInsert(fonts, &fontName, font);
		LOG("Font Count: %d\n", fonts->count);

		pthread_mutex_unlock(&fontsMutex);

		hashMapMoveIterator(&itr);
		hashMapDelete(uploadFontsQueue, &fontName);
	}

	pthread_mutex_unlock(&uploadFontsMutex);

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

	// Upload Audio

	pthread_mutex_lock(&uploadAudioMutex);

	for (HashMapIterator itr = hashMapGetIterator(uploadAudioQueue);
		 !hashMapIteratorAtEnd(itr);)
	{
		AudioFile *audio = hashMapIteratorGetValue(itr);

		pthread_mutex_unlock(&uploadAudioMutex);
		uploadAudioToSoundCard(audio);
		pthread_mutex_lock(&uploadAudioMutex);

		UUID audioName = audio->name;

		pthread_mutex_lock(&audioMutex);

		hashMapInsert(audioFiles, &audioName, audio);
		LOG("Audio Count: %d\n", audioFiles->count);

		pthread_mutex_unlock(&audioMutex);

		hashMapMoveIterator(&itr);
		hashMapDelete(uploadAudioQueue, &audioName);
	}

	pthread_mutex_unlock(&uploadAudioMutex);

	// Upload Particles

	pthread_mutex_lock(&uploadParticlesMutex);

	for (HashMapIterator itr = hashMapGetIterator(uploadParticlesQueue);
		 !hashMapIteratorAtEnd(itr);)
	{
		Particle *particle = hashMapIteratorGetValue(itr);

		pthread_mutex_unlock(&uploadParticlesMutex);
		uploadParticleToGPU(particle);
		pthread_mutex_lock(&uploadParticlesMutex);

		UUID particleName = particle->name;

		pthread_mutex_lock(&particlesMutex);

		hashMapInsert(particles, &particleName, particle);
		LOG("Particle Count: %d\n", particles->count);

		pthread_mutex_unlock(&particlesMutex);

		hashMapMoveIterator(&itr);
		hashMapDelete(uploadParticlesQueue, &particleName);
	}

	pthread_mutex_unlock(&uploadParticlesMutex);
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

	// Free Fonts

	pthread_mutex_lock(&freeFontsMutex);

	for (ListIterator listItr = listGetIterator(&freeFontsQueue);
		 !listIteratorAtEnd(listItr);)
	{
		Font *font = LIST_ITERATOR_GET_ELEMENT(Font, listItr);

		pthread_mutex_unlock(&freeFontsMutex);
		freeFontData(font);
		pthread_mutex_lock(&freeFontsMutex);

		listRemove(&freeFontsQueue, &listItr);
	}

	pthread_mutex_unlock(&freeFontsMutex);

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

	// Free Audio

	pthread_mutex_lock(&freeAudioMutex);

	for (ListIterator listItr = listGetIterator(&freeAudioQueue);
		 !listIteratorAtEnd(listItr);)
	{
		AudioFile *audio = LIST_ITERATOR_GET_ELEMENT(AudioFile, listItr);

		pthread_mutex_unlock(&freeAudioMutex);
		freeAudioData(audio);
		pthread_mutex_lock(&freeAudioMutex);

		listRemove(&freeAudioQueue, &listItr);
	}

	pthread_mutex_unlock(&freeAudioMutex);

	// Free Particles

	pthread_mutex_lock(&freeParticleMutex);

	for (ListIterator listItr = listGetIterator(&freeParticleQueue);
		 !listIteratorAtEnd(listItr);)
	{
		Particle *particle = LIST_ITERATOR_GET_ELEMENT(Particle, listItr);

		pthread_mutex_unlock(&freeParticleMutex);
		freeParticleData(particle);
		pthread_mutex_lock(&freeParticleMutex);

		listRemove(&freeParticleQueue, &listItr);
	}

	pthread_mutex_unlock(&freeParticleMutex);
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

	for (ListIterator listItr = listGetIterator(&freeFontsQueue);
		 !listIteratorAtEnd(listItr);
		 listMoveIterator(&listItr))
	{
		Font *font = LIST_ITERATOR_GET_ELEMENT(Font, listItr);
		hashMapInsert(fonts, &font->name, font);
	}

	listClear(&freeFontsQueue);
	pthread_mutex_destroy(&freeFontsMutex);

	for (ListIterator listItr = listGetIterator(&freeImagesQueue);
		 !listIteratorAtEnd(listItr);
		 listMoveIterator(&listItr))
	{
		Image *image = LIST_ITERATOR_GET_ELEMENT(Image, listItr);
		hashMapInsert(images, &image->name, image);
	}

	listClear(&freeImagesQueue);
	pthread_mutex_destroy(&freeImagesMutex);

	for (ListIterator listItr = listGetIterator(&freeAudioQueue);
		 !listIteratorAtEnd(listItr);
		 listMoveIterator(&listItr))
	{
		AudioFile *audio = LIST_ITERATOR_GET_ELEMENT(AudioFile, listItr);
		hashMapInsert(audioFiles, &audio->name, audio);
	}

	listClear(&freeAudioQueue);
	pthread_mutex_destroy(&freeAudioMutex);

	for (ListIterator listItr = listGetIterator(&freeParticleQueue);
		 !listIteratorAtEnd(listItr);
		 listMoveIterator(&listItr))
	{
		Particle *particle = LIST_ITERATOR_GET_ELEMENT(Particle, listItr);
		hashMapInsert(particles, &particle->name, particle);
	}

	listClear(&freeParticleQueue);
	pthread_mutex_destroy(&freeParticleMutex);

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

	for (HashMapIterator itr = hashMapGetIterator(uploadFontsQueue);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		Font *font = hashMapIteratorGetValue(itr);
		hashMapInsert(fonts, &font->name, font);
	}

	freeHashMap(&uploadFontsQueue);
	pthread_mutex_destroy(&uploadFontsMutex);

	for (HashMapIterator itr = hashMapGetIterator(uploadImagesQueue);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		Image *image = hashMapIteratorGetValue(itr);
		hashMapInsert(images, &image->name, image);
	}

	freeHashMap(&uploadImagesQueue);
	pthread_mutex_destroy(&uploadImagesMutex);

	for (HashMapIterator itr = hashMapGetIterator(uploadAudioQueue);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		AudioFile *audio = hashMapIteratorGetValue(itr);
		hashMapInsert(audioFiles, &audio->name, audio);
	}

	freeHashMap(&uploadAudioQueue);
	pthread_mutex_destroy(&uploadAudioMutex);

	for (HashMapIterator itr = hashMapGetIterator(uploadParticlesQueue);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		Particle *particle = hashMapIteratorGetValue(itr);
		hashMapInsert(particles, &particle->name, particle);
	}

	freeHashMap(&uploadParticlesQueue);
	pthread_mutex_destroy(&uploadParticlesMutex);

	// Resource Loading HashMaps

	freeHashMap(&loadingModels);
	pthread_mutex_destroy(&loadingModelsMutex);

	freeHashMap(&loadingTextures);
	pthread_mutex_destroy(&loadingTexturesMutex);

	freeHashMap(&loadingFonts);
	pthread_mutex_destroy(&loadingFontsMutex);

	freeHashMap(&loadingImages);
	pthread_mutex_destroy(&loadingImagesMutex);

	freeHashMap(&loadingAudio);
	pthread_mutex_destroy(&loadingAudioMutex);

	freeHashMap(&loadingParticles);
	pthread_mutex_destroy(&loadingParticlesMutex);

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
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		freeFontData(hashMapIteratorGetValue(itr));
	}

	freeHashMap(&fonts);
	pthread_mutex_destroy(&fontsMutex);

	for (HashMapIterator itr = hashMapGetIterator(images);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		freeImageData(hashMapIteratorGetValue(itr));
	}

	freeHashMap(&images);
	pthread_mutex_destroy(&imagesMutex);

	for (HashMapIterator itr = hashMapGetIterator(audioFiles);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		freeAudioData(hashMapIteratorGetValue(itr));
	}

	freeHashMap(&audioFiles);
	pthread_mutex_destroy(&audioMutex);

	for (HashMapIterator itr = hashMapGetIterator(particles);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		freeParticleData(hashMapIteratorGetValue(itr));
	}

	freeHashMap(&particles);
	pthread_mutex_destroy(&particlesMutex);
}