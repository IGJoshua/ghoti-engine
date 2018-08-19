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

internal void loadMaterialFolders(UUID name);
internal UUID getMaterialComponentTextureName(
	Material *material,
	MaterialComponentType materialComponentType);
internal int32 loadMaterialTexture(UUID materialName, UUID textureName);

int32 loadMaterial(Material *material, FILE *file)
{
	material->name = readStringAsUUID(file);

	if (strlen(material->name.string) > 0)
	{
		LOG("Loading material (%s)...\n", material->name.string);

		fread(&material->doubleSided, sizeof(bool), 1, file);

		loadMaterialFolders(material->name);

		for (uint32 i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
		{
			MaterialComponent *materialComponent = &material->components[i];
			MaterialComponentType materialComponentType =
				(MaterialComponentType)i;

			materialComponent->texture =
				getMaterialComponentTextureName(
					material,
					materialComponentType);

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

		LOG("Successfully loaded material (%s)\n", material->name.string);
	}

	return 0;
}

int32 loadMaterialTextures(Material *material)
{
	if (strlen(material->name.string) > 0)
	{
		for (uint32 j = 0; j < MATERIAL_COMPONENT_TYPE_COUNT; j++)
		{
			MaterialComponent *materialComponent = &material->components[j];
			if (loadMaterialTexture(
				material->name,
				materialComponent->texture) == -1)
			{
				return -1;
			}
		}
	}

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
	if (!hashMapGetData(&materialFolders, &name))
	{
		List materialNames = createList(sizeof(UUID));

		UUID materialName = idFromName(strtok(name.string, "_"));
		while (strlen(materialName.string) > 0)
		{
			listPushBack(&materialNames, &materialName);
			materialName = idFromName(strtok(NULL, "_"));
		}

		List materialFoldersList = createList(sizeof(char*));

		char *materialFolder = NULL;
		for (ListIterator itr = listGetIterator(&materialNames);
			 !listIteratorAtEnd(itr);
			 listMoveIterator(&itr))
		{
			materialName = *LIST_ITERATOR_GET_ELEMENT(UUID, itr);
			if (!materialFolder)
			{
				materialFolder = malloc(strlen(materialName.string) + 3);
				sprintf(materialFolder, "m_%s", materialName.string);

				char *folder = malloc(strlen(materialFolder) + 1);
				strcpy(folder, materialFolder);

				listPushFront(&materialFoldersList, &folder);
			}
			else
			{
				materialFolder = realloc(
					materialFolder,
					strlen(materialFolder) * 2
					+ strlen(materialName.string) + 3);

				char *folder = malloc(strlen(materialFolder) + 1);
				strcpy(folder, materialFolder);

				sprintf(
					materialFolder,
					"%s/%s_%s",
					folder,
					folder,
					materialName.string);

				free(folder);
				folder = malloc(strlen(materialFolder) + 1);
				strcpy(folder, materialFolder);

				listPushFront(&materialFoldersList, &folder);
			}
		}

		listClear(&materialNames);
		free(materialFolder);

		hashMapInsert(&materialFolders, &name, &materialFoldersList);
	}
}

UUID getMaterialComponentTextureName(
	Material *material,
	MaterialComponentType materialComponentType)
{
	UUID textureName = {};

	char suffix = '\0';
	switch (materialComponentType) {
		case MATERIAL_COMPONENT_TYPE_BASE:
			suffix = 'b';
			break;
		case MATERIAL_COMPONENT_TYPE_EMISSIVE:
			suffix = 'e';
			break;
		case MATERIAL_COMPONENT_TYPE_METALLIC:
			suffix = 'm';
			break;
		case MATERIAL_COMPONENT_TYPE_NORMAL:
			suffix = 'n';
			break;
		case MATERIAL_COMPONENT_TYPE_ROUGHNESS:
			suffix = 'r';
			break;
		default:
			break;
	}

	sprintf(textureName.string, "%s_%c", material->name.string, suffix);

	return textureName;
}

int32 loadMaterialTexture(UUID materialName, UUID textureName)
{
	if (!hashMapGetData(&textures, &textureName))
	{
		List *materialFoldersList = (List*)hashMapGetData(
			&materialFolders,
			&materialName);

		char *fullFilename = NULL;
		for (ListIterator itr = listGetIterator(materialFoldersList);
			 !listIteratorAtEnd(itr);
			 listMoveIterator(&itr))
		{
			char *materialFolder = *LIST_ITERATOR_GET_ELEMENT(char*, itr);

			char *filename = malloc(
				strlen(textureName.string) + strlen(materialFolder) + 24);

			sprintf(
				filename,
				"resources/materials/%s/t_%s",
				materialFolder,
				textureName.string);

			fullFilename = getFullTextureFilename(filename);
			free(filename);

			if (fullFilename)
			{
				break;
			}

			free(fullFilename);
		}

		if (fullFilename)
		{
			if (loadTexture(fullFilename, textureName.string) == -1)
			{
				free(fullFilename);
				return -1;
			}

			free(fullFilename);
		}
	}

	return 0;
}