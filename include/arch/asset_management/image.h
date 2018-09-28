#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

void loadImage(const char *name, bool textureFiltering);
Image getImage(const char *name);
void freeImageData(Image *image);