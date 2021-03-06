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
#include <pthread.h>

extern HashMap materialFolders;
extern pthread_mutex_t materialFoldersMutex;

internal const char materialComponentCharacters[] = {
	'a', 'b', 'e', 'h', 'm', 'n', 'r'
};

int32 loadMaterial(Material *material, FILE *file, const char *modelName)
{
	for (uint32 i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
	{
		MaterialComponent *materialComponent = &material->components[i];
		kmVec3Fill(&materialComponent->value, 1.0f, 1.0f, 1.0f);
	}

	material->name = readStringAsUUID(file);

	if (strlen(material->name.string) > 0)
	{
		ASSET_LOG(
			MODEL,
			modelName,
			"Loading material (%s)...\n",
			material->name.string);

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
			fread(&materialComponent->value.y, sizeof(uint32), 1, file);
			fread(&materialComponent->value.z, sizeof(uint32), 1, file);
		}

		ASSET_LOG(
			MODEL,
			modelName,
			"Successfully loaded material (%s)\n",
			material->name.string);
	}

	return 0;
}

int32 createMaterial(UUID name, Material *material)
{
	LOG("Loading material (%s)...\n", name.string);

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

	LOG("Successfully loaded material (%s)\n", name.string);

	return 0;
}

void loadMaterialFolders(UUID name)
{
	pthread_mutex_lock(&materialFoldersMutex);
	if (!hashMapGetData(materialFolders, &name))
	{
		pthread_mutex_unlock(&materialFoldersMutex);

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

		pthread_mutex_lock(&materialFoldersMutex);
		hashMapInsert(
			materialFolders,
			&fullMaterialName,
			&materialFoldersList);
	}

	pthread_mutex_unlock(&materialFoldersMutex);
}

int32 loadMaterialComponentTexture(
	UUID materialName,
	MaterialComponentType materialComponentType,
	UUID *textureName)
{
	memset(textureName, 0, sizeof(UUID));

	List materialFoldersList = *(List*)hashMapGetData(
		materialFolders,
		&materialName);

	char *fullFilename = NULL;
	for (ListIterator itr = listGetIterator(&materialFoldersList);
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
		loadTexture(fullFilename, textureName->string);
		free(fullFilename);
	}

	return 0;
}
