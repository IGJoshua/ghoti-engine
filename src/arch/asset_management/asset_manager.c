#include "asset_management/asset_manager.h"
#include "asset_management/asset_manager_types.h"
#include "asset_management/model.h"
#include "asset_management/font.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "components/component_types.h"

#include <string.h>

extern HashMap models;
extern HashMap textures;
extern HashMap materialFolders;
extern HashMap fonts;

void initializeAssetManager(void) {
	models = createHashMap(
		sizeof(UUID),
		sizeof(Model),
		MODELS_BUCKET_COUNT,
		(ComparisonOp)&strcmp);

	textures = createHashMap(
		sizeof(UUID),
		sizeof(Texture),
		TEXTURES_BUCKET_COUNT,
		(ComparisonOp)&strcmp);

	materialFolders = createHashMap(
		sizeof(UUID),
		sizeof(List),
		MATERIAL_FOLDERS_BUCKET_COUNT,
		(ComparisonOp)&strcmp);

	fonts = createHashMap(
		sizeof(UUID),
		sizeof(Font),
		FONTS_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
}

void loadAssets(UUID componentID, ComponentDataEntry *entry)
{
	if (!strcmp(componentID.string, "model"))
	{
		loadModel(((ModelComponent*)entry->data)->name);
	}
	else if (!strcmp(componentID.string, "panel"))
	{
		PanelComponent *panelComponent = (PanelComponent*)entry->data;
		loadFont(panelComponent->font, panelComponent->fontSize);
	}
}

void freeAssets(UUID componentID, ComponentDataEntry *entry)
{
	if (!strcmp(componentID.string, "model"))
	{
		freeModel(((ModelComponent*)entry->data)->name);
	}
}

void shutdownAssetManager(void) {
	freeHashMap(&models);
	freeHashMap(&textures);

	for (HashMapIterator itr = hashMapGetIterator(materialFolders);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		List *materialFoldersList = (List*)hashMapIteratorGetValue(itr);
		for (ListIterator listItr = listGetIterator(materialFoldersList);
			 !listIteratorAtEnd(listItr);
			 listMoveIterator(&listItr))
		{
			free(LIST_ITERATOR_GET_ELEMENT(MaterialFolder, listItr)->folder);
		}

		listClear(materialFoldersList);
	}

	freeHashMap(&materialFolders);

	for (HashMapIterator itr = hashMapGetIterator(fonts);
		 !hashMapIteratorAtEnd(itr);)
	{
		Font *font = (Font*)hashMapIteratorGetValue(itr);
		hashMapMoveIterator(&itr);
		freeFont(font);
	}

	freeHashMap(&fonts);
}
