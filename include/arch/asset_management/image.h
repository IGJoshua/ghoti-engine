#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

void loadImage(const char *name);
int32 uploadImageToGPU(Image *image);
Image getImage(const char *name);
void freeImage(const char *name);
void freeImageData(Image *image);