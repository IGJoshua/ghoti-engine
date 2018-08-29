#include "asset_management/asset_manager_types.h"
#include "asset_management/font.h"

#include "core/log.h"

#include "file/utilities.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include "ECS/scene.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING

#define NK_IMPLEMENTATION
#include <nuklear/nuklear.h>

extern HashMap fonts;

internal UUID getFontName(const char *name, uint32 size);

int32 loadFont(const char *name, uint32 size)
{
	int32 error = 0;

	if (!getFont(name, size))
	{
		Font font = {};

		UUID fullName = getFontName(name, size);

		LOG("Loading font (%s)...\n", fullName.string);

		font.name = fullName;

		nk_font_atlas_init_default(&font.atlas);
		nk_font_atlas_begin(&font.atlas);

		char *filename = getFullFilePath(name, "ttf", "resources/fonts");

		font.font = nk_font_atlas_add_from_file(
			&font.atlas,
			filename,
			size,
			NULL);

		free(filename);

		if (!font.font)
		{
			error = -1;
		}
		else
		{
			int32 width, height;
			const void *image = nk_font_atlas_bake(
				&font.atlas,
				&width,
				&height,
				NK_FONT_ATLAS_RGBA32);

			glGenTextures(1, &font.texture);
			glBindTexture(GL_TEXTURE_2D, font.texture);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGBA,
				width,
				height,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				image);

			GLenum glError = glGetError();
			if (glError != GL_NO_ERROR)
			{
				LOG("Error while transferring font onto GPU: %s\n",
					gluErrorString(glError));

				error = -1;
			}

			glBindTexture(GL_TEXTURE_2D, 0);

			if (error != -1)
			{
				nk_font_atlas_end(
					&font.atlas,
					nk_handle_id(font.texture),
					&font.null);
			}
		}

		if (error != -1)
		{
			hashMapInsert(fonts, &font.name, &font);

			LOG("Successfully loaded font (%s)\n", fullName.string);
			LOG("Font Count: %d\n", fonts->count);
		}
		else
		{
			LOG("Failed to load font (%s)\n", fullName.string);
		}
	}

	return error;
}

Font* getFont(const char *name, uint32 size)
{
	Font *font = NULL;
	if (strlen(name) > 0)
	{
		UUID nameID = getFontName(name, size);
		font = hashMapGetData(fonts, &nameID);
	}

	return font;
}

void freeFont(Font *font)
{
	UUID name = font->name;

	LOG("Freeing font (%s)...\n", name.string);

	nk_font_atlas_clear(&font->atlas);
	glDeleteTextures(1, &font->texture);
	hashMapDelete(fonts, &name);

	LOG("Successfully freed font (%s)\n", name.string);
	LOG("Font Count: %d\n", fonts->count);
}

UUID getFontName(const char *name, uint32 size)
{
	UUID fullName = {};
	sprintf(fullName.string, "%s_%dpt", name, size);
	return fullName;
}