#include "asset_management/asset_manager_types.h"
#include "asset_management/particle.h"
#include "asset_management/texture.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include "ECS/scene.h"

#include "file/utilities.h"

#include "renderer/renderer_utilities.h"

extern HashMap particles;

internal void deleteParticle(const char *name);
internal char* getFullParticleFilename(const char *name);

int32 loadParticle(const char *name, int32 spriteWidth, int32 spriteHeight)
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
			particle.refCount = 1;
			particle.spriteWidth = spriteWidth;
			particle.spriteHeight = spriteHeight;

			ILuint devilID;
			error = loadTextureData(
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
					"Error while transferring particle onto GPU");

				if (error != -1)
				{
					glBindTexture(GL_TEXTURE_2D, 0);
					ilDeleteImages(1, &devilID);
				}
			}
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
	else
	{
		particleResource->refCount++;
	}

	return error;
}

Particle* getParticle(const char *name)
{
	Particle *particle = NULL;
	if (strlen(name) > 0)
	{
		UUID nameID = idFromName(name);
		particle = hashMapGetData(particles, &nameID);
	}

	return particle;
}

void deleteParticle(const char *name)
{
	UUID nameID = idFromName(name);
	hashMapDelete(particles, &nameID);
}

void freeParticle(const char *name)
{
	Particle *particle = getParticle(name);
	if (particle)
	{
		if (--particle->refCount == 0)
		{
			LOG("Freeing particle (%s)...\n", name);

			glDeleteTextures(1, &particle->id);
			deleteParticle(name);

			LOG("Successfully freed particle (%s)\n", name);
			LOG("Particle Count: %d\n", particles->count);
		}
	}
}

char* getFullParticleFilename(const char *name)
{
	char *filename = getFullFilePath(name, NULL, "resources/particles");
	char *fullFilename = getFullTextureFilename(filename);
	free(filename);

	return fullFilename;
}