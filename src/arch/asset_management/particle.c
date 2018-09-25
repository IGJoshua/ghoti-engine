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

extern HashMap particles;

extern pthread_mutex_t devilMutex;

internal void deleteParticle(const char *name);
internal char* getFullParticleFilename(const char *name);

int32 loadParticle(
	const char *name,
	uint32 numSprites,
	int32 spriteWidth,
	int32 spriteHeight)
{
	int32 error = 0;

	Particle *particleResource = getParticle(name);
	if (!particleResource)
	{
		Particle particle = {};
		const char *particleName = name;

		char *fullFilename = getFullParticleFilename(name);

		if (!fullFilename)
		{
			error = -1;
		}
		else
		{
			particleName = strrchr(fullFilename, '/');
			if (!particleName)
			{
				particleName = fullFilename;
			}
			else
			{
				particleName += 1;
			}

			LOG("Loading particle (%s)...\n", particleName);

			particle.name = idFromName(name);
			particle.numSprites = numSprites;
			particle.spriteWidth = spriteWidth;
			particle.spriteHeight = spriteHeight;

			pthread_mutex_lock(&devilMutex);

			ILuint devilID;
			// TODO: Change to ASSET_LOG_TYPE_PARTICLE
			error = loadTextureData(
				ASSET_LOG_TYPE_NONE,
				"particle",
				NULL,
				fullFilename,
				TEXTURE_FORMAT_RGBA8,
				&devilID);

			if (error != -1)
			{
				glGenTextures(1, &particle.id);
				glBindTexture(GL_TEXTURE_2D, particle.id);

				particle.width = ilGetInteger(IL_IMAGE_WIDTH);
				particle.height = ilGetInteger(IL_IMAGE_HEIGHT);

				glTexStorage2D(
					GL_TEXTURE_2D,
					1,
					GL_RGBA8,
					particle.width,
					particle.height);

				const GLvoid *particleData = ilGetData();
				glTexSubImage2D(
					GL_TEXTURE_2D,
					0,
					0,
					0,
					particle.width,
					particle.height,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					particleData);

				error = logGLError(
					false,
					"Failed to transfer particle onto GPU");

				if (error != -1)
				{
					glBindTexture(GL_TEXTURE_2D, 0);
					ilDeleteImages(1, &devilID);
				}
			}

			pthread_mutex_unlock(&devilMutex);
		}

		if (error != - 1)
		{
			hashMapInsert(particles, &particle.name, &particle);

			LOG("Successfully loaded particle (%s)\n", particleName);
			LOG("Particle Count: %d\n", particles->count);
		}
		else
		{
			LOG("Failed to load particle (%s)\n", particleName);
		}

		free(fullFilename);
	}

	return error;
}

Particle* getParticle(const char *name)
{
	Particle *particle = NULL;
	if (strlen(name) > 0)
	{
		UUID particleName = idFromName(name);
		particle = hashMapGetData(particles, &particleName);
	}

	return particle;
}

void deleteParticle(const char *name)
{
	UUID particleName = idFromName(name);
	hashMapDelete(particles, &particleName);
}

void freeParticle(const char *name)
{
	Particle *particle = getParticle(name);
	if (particle)
	{
		LOG("Freeing particle (%s)...\n", name);

		glDeleteTextures(1, &particle->id);
		deleteParticle(name);

		LOG("Successfully freed particle (%s)\n", name);
		LOG("Particle Count: %d\n", particles->count);
	}
}

char* getFullParticleFilename(const char *name)
{
	char *filename = getFullFilePath(name, NULL, "resources/particles");
	char *fullFilename = getFullTextureFilename(filename);
	free(filename);

	return fullFilename;
}