#include "asset_management/asset_manager_types.h"
#include "asset_management/image.h"
#include "asset_management/texture.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include "ECS/scene.h"

#include "file/utilities.h"

#include "renderer/renderer_utilities.h"

extern HashMap images;

internal void deleteImage(const char *name);
internal char* getFullImageFilename(const char *name);

int32 loadImage(const char *name)
{
	int32 error = 0;

	Image *imageResource = getImage(name);
	if (!imageResource)
	{
		Image image = {};
		const char *imageName = name;

		char *fullFilename = getFullImageFilename(name);

		if (!fullFilename)
		{
			error = -1;
		}
		else
		{
			imageName = strrchr(fullFilename, '/');
			if (!imageName)
			{
				imageName = fullFilename;
			}
			else
			{
				imageName += 1;
			}

			LOG("Loading image (%s)...\n", imageName);

			image.name = idFromName(name);
			image.refCount = 1;

			ILuint devilID;

			// TODO: Change to ASSET_LOG_TYPE_IMAGE
			error = loadTextureData(
				ASSET_LOG_TYPE_NONE,
				"image",
				NULL,
				fullFilename,
				TEXTURE_FORMAT_RGBA8,
				&devilID);

			if (error != -1)
			{
				glGenTextures(1, &image.id);
				glBindTexture(GL_TEXTURE_2D, image.id);

				image.width = ilGetInteger(IL_IMAGE_WIDTH);
				image.height = ilGetInteger(IL_IMAGE_HEIGHT);

				glTexStorage2D(
					GL_TEXTURE_2D,
					1,
					GL_RGBA8,
					image.width,
					image.height);

				const GLvoid *imageData = ilGetData();
				glTexSubImage2D(
					GL_TEXTURE_2D,
					0,
					0,
					0,
					image.width,
					image.height,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					imageData);

				error = logGLError(
					false,
					"Error while transferring image onto GPU");

				if (error != -1)
				{
					glBindTexture(GL_TEXTURE_2D, 0);
					ilDeleteImages(1, &devilID);
				}
			}
		}

		if (error != - 1)
		{
			hashMapInsert(images, &image.name, &image);

			LOG("Successfully loaded image (%s)\n", imageName);
			LOG("Image Count: %d\n", images->count);
		}
		else
		{
			LOG("Failed to load image (%s)\n", imageName);
		}

		free(fullFilename);
	}
	else
	{
		imageResource->refCount++;
	}

	return error;
}

Image* getImage(const char *name)
{
	Image *image = NULL;
	if (strlen(name) > 0)
	{
		UUID nameID = idFromName(name);
		image = hashMapGetData(images, &nameID);
	}

	return image;
}

void deleteImage(const char *name)
{
	UUID nameID = idFromName(name);
	hashMapDelete(images, &nameID);
}

void freeImage(const char *name)
{
	Image *image = getImage(name);
	if (image)
	{
		if (--image->refCount == 0)
		{
			LOG("Freeing image (%s)...\n", name);

			glDeleteTextures(1, &image->id);
			deleteImage(name);

			LOG("Successfully freed image (%s)\n", name);
			LOG("Image Count: %d\n", images->count);
		}
	}
}

char* getFullImageFilename(const char *name)
{
	char *filename = getFullFilePath(name, NULL, "resources/images");
	char *fullFilename = getFullTextureFilename(filename);
	free(filename);

	return fullFilename;
}