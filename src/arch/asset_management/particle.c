#include "asset_management/asset_manager_types.h"
#include "asset_management/particle.h"
#include "asset_management/texture.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include "ECS/scene.h"

#include "file/utilities.h"

#include "renderer/renderer_utilities.h"

#include <pthread.h>

typedef struct particle_thread_args_t
{
	char *name;
	uint32 numSprites;
	int32 spriteWidth;
	int32 spriteHeight;
} ParticleThreadArgs;

extern Config config;

extern HashMap particles;
extern pthread_mutex_t particlesMutex;

extern HashMap loadingParticles;
extern pthread_mutex_t loadingParticlesMutex;

extern HashMap uploadParticlesQueue;
extern pthread_mutex_t uploadParticlesMutex;

extern uint32 assetThreadCount;
extern pthread_mutex_t assetThreadsMutex;
extern pthread_cond_t assetThreadsCondition;

extern pthread_mutex_t devilMutex;

internal void* acquireParticleThread(void *arg);
internal void* loadParticleThread(void *arg);

internal char* getFullParticleFilename(const char *name);

void loadParticle(
	const char *name,
	uint32 numSprites,
	int32 spriteWidth,
	int32 spriteHeight)
{
	ParticleThreadArgs *arg = malloc(sizeof(ParticleThreadArgs));

	arg->name = calloc(1, strlen(name) + 1);
	strcpy(arg->name, name);

	arg->numSprites = numSprites;
	arg->spriteWidth = spriteWidth;
	arg->spriteHeight = spriteHeight;

	pthread_t acquisitionThread;
	pthread_create(
		&acquisitionThread,
		NULL,
		&acquireParticleThread,
		(void*)arg);
	pthread_detach(acquisitionThread);
}

void* acquireParticleThread(void *arg)
{
	pthread_mutex_lock(&assetThreadsMutex);

	while (assetThreadCount == config.assetsConfig.maxThreadCount)
	{
		pthread_cond_wait(&assetThreadsCondition, &assetThreadsMutex);
	}

	assetThreadCount++;

	pthread_mutex_unlock(&assetThreadsMutex);
	pthread_cond_broadcast(&assetThreadsCondition);

	pthread_t loadingThread;
	pthread_create(&loadingThread, NULL, &loadParticleThread, arg);
	pthread_detach(loadingThread);

	EXIT_THREAD(NULL);
}

void* loadParticleThread(void *arg)
{
	int32 error = 0;

	ParticleThreadArgs *threadArgs = arg;
	char *name = threadArgs->name;
	uint32 numSprites = threadArgs->numSprites;
	int32 spriteWidth = threadArgs->spriteWidth;
	int32 spriteHeight = threadArgs->spriteHeight;

	UUID nameID = idFromName(name);

	pthread_mutex_lock(&particlesMutex);
	Particle *particleResource = hashMapGetData(particles, &nameID);

	if (!particleResource)
	{
		pthread_mutex_unlock(&particlesMutex);
		pthread_mutex_lock(&loadingParticlesMutex);

		if (hashMapGetData(loadingParticles, &nameID))
		{
			error = 1;
		}

		pthread_mutex_unlock(&loadingParticlesMutex);

		if (error != 1)
		{
			pthread_mutex_lock(&uploadParticlesMutex);

			if (hashMapGetData(uploadParticlesQueue, &nameID))
			{
				error = 1;
			}

			pthread_mutex_unlock(&uploadParticlesMutex);
		}

		if (error != 1)
		{
			bool loading = true;
			pthread_mutex_lock(&loadingParticlesMutex);
			hashMapInsert(loadingParticles, &nameID, &loading);
			pthread_mutex_unlock(&loadingParticlesMutex);

			char *fullFilename = getFullParticleFilename(name);
			if (!fullFilename)
			{
				error = -1;
			}
			else
			{
				const char *particleName = strrchr(fullFilename, '/');
				if (!particleName)
				{
					particleName = fullFilename;
				}
				else
				{
					particleName += 1;
				}

				ASSET_LOG(
					PARTICLE,
					name,
					"Loading particle (%s)...\n",
					particleName);

				Particle particle = {};

				particle.name = idFromName(name);
				particle.lifetime = config.assetsConfig.minParticleLifetime;
				particle.numSprites = numSprites;
				particle.spriteWidth = spriteWidth;
				particle.spriteHeight = spriteHeight;

				pthread_mutex_lock(&devilMutex);

				error = loadTextureData(
					ASSET_LOG_TYPE_PARTICLE,
					"particle",
					name,
					fullFilename,
					TEXTURE_FORMAT_RGBA8,
					&particle.devilID);
				ilBindImage(0);

				pthread_mutex_unlock(&devilMutex);

				if (error != - 1)
				{
					pthread_mutex_lock(&uploadParticlesMutex);
					hashMapInsert(uploadParticlesQueue, &nameID, &particle);
					pthread_mutex_unlock(&uploadParticlesMutex);

					pthread_mutex_lock(&loadingParticlesMutex);
					hashMapDelete(loadingParticles, &nameID);
					pthread_mutex_unlock(&loadingParticlesMutex);

					ASSET_LOG(
						PARTICLE,
						name,
						"Successfully loaded particle (%s)\n",
						particleName);
				}

				ASSET_LOG_COMMIT(PARTICLE, name);
			}

			free(fullFilename);
		}
	}
	else
	{
		pthread_mutex_unlock(&particlesMutex);
	}

	free(arg);
	free(name);

	pthread_mutex_lock(&assetThreadsMutex);
	assetThreadCount--;
	pthread_mutex_unlock(&assetThreadsMutex);
	pthread_cond_broadcast(&assetThreadsCondition);

	EXIT_THREAD(NULL);
}

int32 uploadParticleToGPU(Particle *particle)
{
	LOG("Transferring particle (%s) onto GPU...\n", particle->name.string);

	pthread_mutex_lock(&devilMutex);

	ilBindImage(particle->devilID);

	glGenTextures(1, &particle->id);
	glBindTexture(GL_TEXTURE_2D, particle->id);

	particle->width = ilGetInteger(IL_IMAGE_WIDTH);
	particle->height = ilGetInteger(IL_IMAGE_HEIGHT);

	const GLvoid *spriteSheetData = ilGetData();
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		particle->width,
		particle->height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		spriteSheetData);

	ilDeleteImages(1, &particle->devilID);
	ilBindImage(0);

	pthread_mutex_unlock(&devilMutex);

	int32 error = logGLError(false, "Failed to transfer particle onto GPU");

	if (error != -1)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER,
			GL_LINEAR);
		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		LOG("Successfully transferred particle (%s) onto GPU\n",
			particle->name.string);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	return error;
}

Particle getParticle(const char *name)
{
	Particle particle = {};

	if (strlen(name) > 0)
	{
		UUID particleName = idFromName(name);

		pthread_mutex_lock(&particlesMutex);

		Particle *particleResource = hashMapGetData(particles, &particleName);
		if (particleResource)
		{
			particleResource->lifetime =
				config.assetsConfig.minParticleLifetime;
			particle = *particleResource;
		}

		pthread_mutex_unlock(&particlesMutex);
	}

	return particle;
}

void freeParticleData(Particle *particle)
{
	LOG("Freeing particle (%s)...\n", particle->name.string);

	pthread_mutex_lock(&devilMutex);
	ilDeleteImages(1, &particle->devilID);
	pthread_mutex_unlock(&devilMutex);

	glDeleteTextures(1, &particle->id);

	LOG("Successfully freed particle (%s)\n", particle->name.string);
}

char* getFullParticleFilename(const char *name)
{
	char *filename = getFullFilePath(name, NULL, "resources/particles");
	char *fullFilename = getFullTextureFilename(filename);
	free(filename);

	return fullFilename;
}