#include "asset_management/asset_manager.h"
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

EXTERN_ASSET_VARIABLES(fonts, Fonts);
EXTERN_ASSET_MANAGER_VARIABLES;

extern int32 viewportHeight;

INTERNAL_ASSET_THREAD_VARIABLES(Font);

internal uint32 getFontPixelSize(real32 size, bool autoScaling);
internal UUID getFontName(const char *name, real32 size, bool autoScaling);

void loadFont(const char *name, real32 size, bool autoScaling)
{
	FontThreadArgs *arg = malloc(sizeof(FontThreadArgs));

	arg->name = calloc(1, strlen(name) + 1);
	strcpy(arg->name, name);

	arg->size = size;
	arg->autoScaling = autoScaling;

	bool skip = false;

	UUID fontName = getFontName(name, size, autoScaling);

	pthread_mutex_lock(&fontsMutex);
	if (!hashMapGetData(fonts, &fontName))
	{
		pthread_mutex_unlock(&fontsMutex);
		pthread_mutex_lock(&loadingFontsMutex);

		if (hashMapGetData(loadingFonts, &fontName))
		{
			skip = true;
		}

		pthread_mutex_unlock(&loadingFontsMutex);

		if (!skip)
		{
			pthread_mutex_lock(&uploadFontsMutex);

			if (hashMapGetData(uploadFontsQueue, &fontName))
			{
				skip = true;
			}

			pthread_mutex_unlock(&uploadFontsMutex);
		}

		if (!skip)
		{
			START_ACQUISITION_THREAD(Font, arg);
		}
	}
	else
	{
		pthread_mutex_unlock(&fontsMutex);
	}
}

ACQUISITION_THREAD(Font);

void* loadFontThread(void *arg)
{
	int32 error = 0;

	FontThreadArgs *threadArgs = arg;
	char *name = threadArgs->name;
	real32 size = threadArgs->size;
	bool autoScaling = threadArgs->autoScaling;

	UUID fontName = getFontName(name, size, autoScaling);

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

	pthread_mutex_lock(&loadingFontsMutex);
	hashMapDelete(loadingFonts, &fontName);
	pthread_mutex_unlock(&loadingFontsMutex);

	free(arg);
	free(name);

	EXIT_LOADING_THREAD;
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

GET_ASSET_FUNCTION(
	font,
	fonts,
	Font,
	getFont(const char *name, real32 size, bool autoScaling),
	getFontName(name, size, autoScaling));

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