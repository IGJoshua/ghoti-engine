#include "asset_management/asset_manager.h"
#include "asset_management/material.h"
#include "asset_management/texture.h"

#include "renderer/renderer_types.h"

#include "core/log.h"

#include "file/utilities.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/scene.h"

#include <string.h>

extern HashMap textures;
extern HashMap materialFolders;
extern bool asyncAssetLoading;

internal const char materialComponentCharacters[] = {
	'b', 'e', 'm', 'n', 'r'
};

int32 loadMaterial(Material *material, FILE *file)
{
	material->name = readStringAsUUID(file);

	if (strlen(material->name.string) > 0)
	{
		ASSET_LOG("Loading material (%s)...\n", material->name.string);

		fread(&material->doubleSided, sizeof(bool), 1, file);

		loadMaterialFolders(material->name);

		for (uint32 i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
		{
			MaterialComponent *materialComponent = &material->components[i];
			MaterialComponentType materialComponentType =
				(MaterialComponentType)i;

			if (loadMaterialComponentTexture(
				material->name,
				materialComponentType,
				&materialComponent->texture) == -1)
			{
				return -1;
			}

			fread(&materialComponent->value.x, sizeof(uint32), 1, file);
			switch (materialComponentType)
			{
				case MATERIAL_COMPONENT_TYPE_BASE:
				case MATERIAL_COMPONENT_TYPE_EMISSIVE:
					fread(&materialComponent->value.y, sizeof(uint32), 1, file);
					fread(&materialComponent->value.z, sizeof(uint32), 1, file);
					break;
				default:
					materialComponent->value.y = materialComponent->value.x;
					materialComponent->value.z = materialComponent->value.y;
					break;
			}
		}

		ASSET_LOG("Successfully loaded material (%s)\n", material->name.string);
	}

	return 0;
}

int32 createMaterial(UUID name, Material *material)
{
	ASSET_LOG("Loading material (%s)...\n", name.string);

	material->name = name;
	material->doubleSided = false;

	loadMaterialFolders(material->name);

	for (uint32 i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
	{
		MaterialComponent *materialComponent = &material->components[i];
		MaterialComponentType materialComponentType =
			(MaterialComponentType)i;

		if (loadMaterialComponentTexture(
			material->name,
			materialComponentType,
			&materialComponent->texture) == -1)
		{
			return -1;
		}

		kmVec3Fill(&materialComponent->value, 1.0f, 1.0f, 1.0f);
	}

	ASSET_LOG("Successfully loaded material (%s)\n", name.string);

	return 0;
}

void freeMaterial(Material *material)
{
	for (uint32 i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
	{
		freeTexture(material->components[i].texture);
	}
}

void loadMaterialFolders(UUID name)
{
	if (!hashMapGetData(materialFolders, &name))
	{
		List materialNames = createList(sizeof(UUID));

		UUID fullMaterialName = name;
		UUID materialName = idFromName(strtok(fullMaterialName.string, "_"));
		while (strlen(materialName.string) > 0)
		{
			listPushBack(&materialNames, &materialName);
			materialName = idFromName(strtok(NULL, "_"));
		}

		List materialFoldersList = createList(sizeof(MaterialFolder));

		char *materialFolderPath = NULL;
		fullMaterialName = idFromName("");
		for (ListIterator itr = listGetIterator(&materialNames);
			 !listIteratorAtEnd(itr);
			 listMoveIterator(&itr))
		{
			materialName = *LIST_ITERATOR_GET_ELEMENT(UUID, itr);
			if (!materialFolderPath)
			{
				materialFolderPath = malloc(strlen(materialName.string) + 3);
				sprintf(materialFolderPath, "m_%s", materialName.string);

				MaterialFolder materialFolder;

				materialFolder.folder = malloc(strlen(materialFolderPath) + 1);
				strcpy(materialFolder.folder, materialFolderPath);

				fullMaterialName = materialName;
				materialFolder.name = fullMaterialName;

				listPushFront(&materialFoldersList, &materialFolder);
			}
			else
			{
				MaterialFolder materialFolder;

				name = fullMaterialName;
				sprintf(
					fullMaterialName.string,
					"%s_%s",
					name.string,
					materialName.string);

				materialFolder.name = fullMaterialName;

				char *folder = malloc(strlen(materialFolderPath) + 1);
				strcpy(folder, materialFolderPath);

				materialFolderPath = realloc(
					materialFolderPath,
					strlen(materialFolderPath) +
					strlen(fullMaterialName.string) + 4);

				sprintf(
					materialFolderPath,
					"%s/m_%s",
					folder,
					fullMaterialName.string);

				free(folder);

				materialFolder.folder = malloc(strlen(materialFolderPath) + 1);
				strcpy(materialFolder.folder, materialFolderPath);

				listPushFront(&materialFoldersList, &materialFolder);
			}
		}

		listClear(&materialNames);
		free(materialFolderPath);

		hashMapInsert(
			materialFolders,
			&fullMaterialName,
			&materialFoldersList);
	}
}

int32 loadMaterialComponentTexture(
	UUID materialName,
	MaterialComponentType materialComponentType,
	UUID *textureName)
{
	memset(textureName, 0, sizeof(UUID));

	List *materialFoldersList = (List*)hashMapGetData(
		materialFolders,
		&materialName);

	char *fullFilename = NULL;
	for (ListIterator itr = listGetIterator(materialFoldersList);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		MaterialFolder *materialFolder =
			LIST_ITERATOR_GET_ELEMENT(MaterialFolder, itr);

		char *filename = malloc(
			strlen(materialFolder->folder) +
			strlen(materialFolder->name.string) + 26);

		sprintf(
			filename,
			"resources/materials/%s/t_%s_%c",
			materialFolder->folder,
			materialFolder->name.string,
			materialComponentCharacters[materialComponentType]);

		fullFilename = getFullTextureFilename(filename);
		free(filename);

		if (fullFilename)
		{
			sprintf(
				textureName->string,
				"%s_%c",
				materialFolder->name.string,
				materialComponentCharacters[materialComponentType]);

			break;
		}

		free(fullFilename);
	}

	if (fullFilename)
	{
		if (asyncAssetLoading)
		{
			loadAssetAsync(
				ASSET_TYPE_TEXTURE,
				textureName->string,
				fullFilename);
		}
		else
		{
			if (loadTexture(fullFilename, textureName->string) == -1)
			{
				free(fullFilename);
				return -1;
			}
		}

		free(fullFilename);
	}

	return 0;
}
