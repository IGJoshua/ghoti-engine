#include "asset_management/material.h"
#include "asset_management/texture.h"

#include "file/utilities.h"

#include <malloc.h>
#include <string.h>

internal const char *getMaterialComponentName(
	MaterialComponentType materialComponentType);

int32 loadMaterial(Material *material, FILE *file)
{
	material->type = MATERIAL_TYPE_DEBUG;
	material->name = readString(file);

	printf("Loading subset material (%s)...\n", material->name);

	uint32 numMaterialComponents;
	fread(&numMaterialComponents, sizeof(uint32), 1, file);

	memset(
		&material->components,
		0,
		sizeof(MaterialComponent) * MATERIAL_COMPONENT_TYPE_COUNT);

	for (uint32 i = 0; i < numMaterialComponents; i++)
	{
		int8 materialComponentType;
		fread(&materialComponentType, sizeof(int8), 1, file);

		const char *materialComponentName =
			getMaterialComponentName(
				(MaterialComponentType)materialComponentType);

		printf("Loading %s material component...\n", materialComponentName);

		MaterialComponent *materialComponent =
			&material->components[(uint32)materialComponentType];

		switch ((MaterialComponentType)materialComponentType)
		{
			case MATERIAL_COMPONENT_TYPE_DIFFUSE:
			case MATERIAL_COMPONENT_TYPE_SPECULAR:
			case MATERIAL_COMPONENT_TYPE_NORMAL:
			case MATERIAL_COMPONENT_TYPE_EMISSIVE:
				materialComponent->texture = readString(file);
				fread(&materialComponent->uvMap, sizeof(uint32), 1, file);
				break;
			default:
				break;
		}

		switch ((MaterialComponentType)materialComponentType)
		{
			case MATERIAL_COMPONENT_TYPE_NORMAL:
				break;
			default:
				fread(&materialComponent->value.x, sizeof(real32), 1, file);
				fread(&materialComponent->value.y, sizeof(real32), 1, file);
				fread(&materialComponent->value.z, sizeof(real32), 1, file);
				break;
		}

		switch ((MaterialComponentType)materialComponentType)
		{
			case MATERIAL_COMPONENT_TYPE_SPECULAR:
				fread(&materialComponent->value.w, sizeof(real32), 1, file);
				break;
			default:
				break;
		}

		printf("Successfully loaded %s material component\n",
			materialComponentName);
	}

	printf("Successfully loaded subset material (%s)\n", material->name);

	return 0;
}

const char *getMaterialComponentName(
	MaterialComponentType materialComponentType)
{
	switch (materialComponentType)
	{
		case MATERIAL_COMPONENT_TYPE_DIFFUSE:
			return "diffuse";
		case MATERIAL_COMPONENT_TYPE_SPECULAR:
			return "specular";
		case MATERIAL_COMPONENT_TYPE_NORMAL:
			return "normal";
		case MATERIAL_COMPONENT_TYPE_EMISSIVE:
			return "emissive";
		case MATERIAL_COMPONENT_TYPE_AMBIENT:
			return "ambient";
		default:
			break;
	}

	return NULL;
}

int32 freeMaterial(Material *material)
{
	int32 error = 0;

	free(material->name);

	for (uint32 i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
	{
		char *texture = material->components[i].texture;

		if (texture)
		{
			if (strlen(texture) > 0)
			{
				error = freeTexture(texture);
			}

			free(texture);

			if (error == -1)
			{
				break;
			}
		}
	}

	return error;
}