#include "asset_management/asset_manager_types.h"
#include "asset_management/font.h"

#include "core/log.h"

#include "file/utilities.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include "ECS/scene.h"

#include "renderer/renderer_utilities.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING

#define NK_IMPLEMENTATION
#include <nuklear/nuklear.h>

#include <pthread.h>

typedef struct font_thread_args_t
{
	char *name;
	real32 size;
	bool autoScaling;
} FontThreadArgs;

extern Config config;

extern HashMap fonts;
extern pthread_mutex_t fontsMutex;

extern HashMap loadingFonts;
extern pthread_mutex_t loadingFontsMutex;

extern HashMap uploadFontsQueue;
extern pthread_mutex_t uploadFontsMutex;

extern uint32 assetThreadCount;
extern pthread_mutex_t assetThreadsMutex;
extern pthread_cond_t assetThreadsCondition;

extern int32 viewportHeight;

internal void* acquireFontThread(void *arg);
internal void* loadFontThread(void *arg);

internal uint32 getFontPixelSize(real32 size, bool autoScaling);
internal UUID getFontName(const char *name, real32 size, bool autoScaling);

void loadFont(const char *name, real32 size, bool autoScaling)
{
	FontThreadArgs *arg = malloc(sizeof(FontThreadArgs));

	arg->name = calloc(1, strlen(name) + 1);
	strcpy(arg->name, name);

	arg->size = size;
	arg->autoScaling = autoScaling;

	pthread_t acquisitionThread;
	pthread_create(&acquisitionThread, NULL, &acquireFontThread, (void*)arg);
	pthread_detach(acquisitionThread);
}

void* acquireFontThread(void *arg)
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
	pthread_create(&loadingThread, NULL, &loadFontThread, arg);
	pthread_detach(loadingThread);

	EXIT_THREAD(NULL);
}

void* loadFontThread(void *arg)
{
	int32 error = 0;

	FontThreadArgs *threadArgs = arg;
	char *name = threadArgs->name;
	real32 size = threadArgs->size;
	bool autoScaling = threadArgs->autoScaling;

	UUID fontName = getFontName(name, size, autoScaling);

	pthread_mutex_lock(&fontsMutex);
	Font *fontResource = hashMapGetData(fonts, &fontName);

	if (!fontResource)
	{
		pthread_mutex_unlock(&fontsMutex);
		pthread_mutex_lock(&loadingFontsMutex);

		if (hashMapGetData(loadingFonts, &fontName))
		{
			error = 1;
		}

		pthread_mutex_unlock(&loadingFontsMutex);

		if (error != 1)
		{
			pthread_mutex_lock(&uploadFontsMutex);

			if (hashMapGetData(uploadFontsQueue, &fontName))
			{
				error = 1;
			}

			pthread_mutex_unlock(&uploadFontsMutex);
		}

		if (error != 1)
		{
			bool loading = true;
			pthread_mutex_lock(&loadingFontsMutex);
			hashMapInsert(loadingFonts, &fontName, &loading);
			pthread_mutex_unlock(&loadingFontsMutex);

			ASSET_LOG(
				FONT,
				fontName.string,
				"Loading font (%s)...\n",
				fontName.string);

			Font font = {};

			font.name = fontName;
			font.lifetime = config.assetsConfig.minFontLifetime;

			nk_font_atlas_init_default(&font.atlas);
			nk_font_atlas_begin(&font.atlas);

			char *filename = getFullFilePath(name, "ttf", "resources/fonts");

			font.font = nk_font_atlas_add_from_file(
				&font.atlas,
				filename,
				getFontPixelSize(size, autoScaling),
				NULL);

			free(filename);

			if (!font.font)
			{
				error = -1;
			}
			else
			{
				font.textureData = nk_font_atlas_bake(
					&font.atlas,
					&font.textureWidth,
					&font.textureHeight,
					NK_FONT_ATLAS_RGBA32);
			}

			if (error != - 1)
			{
				pthread_mutex_lock(&uploadFontsMutex);
				hashMapInsert(uploadFontsQueue, &fontName, &font);
				pthread_mutex_unlock(&uploadFontsMutex);

				pthread_mutex_lock(&loadingFontsMutex);
				hashMapDelete(loadingFonts, &fontName);
				pthread_mutex_unlock(&loadingFontsMutex);

				ASSET_LOG(
					FONT,
					fontName.string,
					"Successfully loaded font (%s)\n",
					fontName.string);
			}
			else
			{
				ASSET_LOG(
					FONT,
					fontName.string,
					"Failed to load font (%s)\n",
					fontName.string);
			}

			ASSET_LOG_COMMIT(FONT, fontName.string);
		}
	}
	else
	{
		pthread_mutex_unlock(&fontsMutex);
	}

	free(arg);
	free(name);

	pthread_mutex_lock(&assetThreadsMutex);
	assetThreadCount--;
	pthread_mutex_unlock(&assetThreadsMutex);
	pthread_cond_broadcast(&assetThreadsCondition);

	EXIT_THREAD(NULL);
}

int32 uploadFontToGPU(Font *font)
{
	LOG("Transferring font (%s) onto GPU...\n", font->name.string);

	glGenTextures(1, &font->textureID);
	glBindTexture(GL_TEXTURE_2D, font->textureID);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		font->textureWidth,
		font->textureHeight,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		font->textureData);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	nk_font_atlas_end(
		&font->atlas,
		nk_handle_id(font->textureID),
		&font->null);

	int32 error = logGLError(false, "Failed to transfer font onto GPU");

	if (error != -1)
	{
		LOG("Successfully transferred font (%s) onto GPU\n", font->name.string);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	return error;
}

Font getFont(const char *name, real32 size, bool autoScaling)
{
	Font font = {};
	if (strlen(name) > 0)
	{
		UUID fontName = getFontName(name, size, autoScaling);

		pthread_mutex_lock(&fontsMutex);

		Font *fontResource = hashMapGetData(fonts, &fontName);
		if (fontResource)
		{
			fontResource->lifetime = config.assetsConfig.minFontLifetime;
			font = *fontResource;
		}

		pthread_mutex_unlock(&fontsMutex);
	}

	return font;
}

void freeFontData(Font *font)
{
	LOG("Freeing font (%s)...\n", font->name.string);

	nk_font_atlas_clear(&font->atlas);
	glDeleteTextures(1, &font->textureID);

	LOG("Successfully freed font (%s)\n", font->name.string);
}

uint32 getFontPixelSize(real32 size, bool autoScaling)
{
	int32 pixelSize = size * (autoScaling ? viewportHeight : 1.0f);
	if (pixelSize < 1)
	{
		pixelSize = 1;
	}

	return pixelSize;
}

UUID getFontName(const char *name, real32 size, bool autoScaling)
{
	UUID fullName = {};
	sprintf(
		fullName.string,
		"%s_%dpx",
		name,
		getFontPixelSize(size, autoScaling));
	return fullName;
}