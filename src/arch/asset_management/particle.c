#include "asset_management/asset_manager.h"
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
	uint32 rows;
	uint32 columns;
	bool textureFiltering;
} ParticleThreadArgs;

extern Config config;

EXTERN_ASSET_VARIABLES(particles, Particles);
EXTERN_ASSET_MANAGER_VARIABLES;

INTERNAL_ASSET_THREAD_VARIABLES(Particle);

internal char* getFullParticleFilename(const char *name);

void loadParticle(
	const char *name,
	uint32 numSprites,
	uint32 rows,
	uint32 columns,
	bool textureFiltering)
{
	ParticleThreadArgs *arg = malloc(sizeof(ParticleThreadArgs));

	arg->name = calloc(1, strlen(name) + 1);
	strcpy(arg->name, name);

	arg->numSprites = numSprites;
	arg->rows = rows;
	arg->columns = columns;
	arg->textureFiltering = textureFiltering;

	bool skip = false;

	UUID nameID = idFromName(name);

	pthread_mutex_lock(&particlesMutex);
	if (!hashMapGetData(particles, &nameID))
	{
		pthread_mutex_unlock(&particlesMutex);
		pthread_mutex_lock(&loadingParticlesMutex);

		if (hashMapGetData(loadingParticles, &nameID))
		{
			skip = true;
		}

		pthread_mutex_unlock(&loadingParticlesMutex);

		if (!skip)
		{
			pthread_mutex_lock(&uploadParticlesMutex);

			if (hashMapGetData(uploadParticlesQueue, &nameID))
			{
				skip = true;
			}

			pthread_mutex_unlock(&uploadParticlesMutex);
		}

		if (!skip)
		{
			START_ACQUISITION_THREAD(
				particle,
				Particle,
				Particles,
				arg,
				nameID);
			return;
		}
	}
	else
	{
		pthread_mutex_unlock(&particlesMutex);
	}

	free(arg->name);
	free(arg);
}

ACQUISITION_THREAD(Particle);

void* loadParticleThread(void *arg)
{
	int32 error = 0;

	ParticleThreadArgs *threadArgs = arg;
	char *name = threadArgs->name;
	uint32 numSprites = threadArgs->numSprites;
	uint32 rows = threadArgs->rows;
	uint32 columns = threadArgs->columns;
	bool textureFiltering = threadArgs->textureFiltering;

	UUID nameID = idFromName(name);

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
		particle.textureFiltering = textureFiltering;
		particle.numSprites = numSprites == 0 ? 1 : numSprites;
		particle.spriteUVs = malloc(numSprites * sizeof(kmVec2));

		error = loadTextureData(
			ASSET_LOG_TYPE_PARTICLE,
			"particle",
			name,
			fullFilename,
			4,
			&particle.data);

		if (error != - 1)
		{
			real64 spriteSize[2];
			spriteSize[0] = 1.0 / columns;
			spriteSize[1] = 1.0 / rows;

			kmVec2Fill(
				&particle.spriteSize,
				spriteSize[0] - (0.5 / particle.data.width),
				spriteSize[1] - (0.5 / particle.data.height));

			uint32 spriteUVIndex = 0;
			for (real64 v = 0.0;
					v < 1.0 - spriteSize[1] / 2;
					v += spriteSize[1])
			{
				for (real64 u = 0.0;
						u < 1.0 - spriteSize[0] / 2;
						u += spriteSize[0])
				{
					kmVec2Fill(
						&particle.spriteUVs[spriteUVIndex++],
						u,
						v);

					if (spriteUVIndex == particle.numSprites)
					{
						break;
					}
				}

				if (spriteUVIndex == particle.numSprites)
				{
					break;
				}
			}

			pthread_mutex_lock(&uploadParticlesMutex);
			hashMapInsert(uploadParticlesQueue, &nameID, &particle);
			pthread_mutex_unlock(&uploadParticlesMutex);

			ASSET_LOG(
				PARTICLE,
				name,
				"Successfully loaded particle (%s)\n",
				particleName);
		}

		ASSET_LOG_COMMIT(PARTICLE, name);

		pthread_mutex_lock(&loadingParticlesMutex);
		hashMapDelete(loadingParticles, &nameID);
		pthread_mutex_unlock(&loadingParticlesMutex);
	}

	free(fullFilename);

	free(arg);
	free(name);

	EXIT_LOADING_THREAD;
}

GET_ASSET_FUNCTION(
	particle,
	particles,
	Particle,
	getParticle(const char *name),
	idFromName(name));

void freeParticleData(Particle *particle)
{
	LOG("Freeing particle (%s)...\n", particle->name.string);

	free(particle->spriteUVs);
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