#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

void initializeCubemapImporter(void);
void importCubemap(Cubemap *cubemap);
void shutdownCubemapImporter(void);