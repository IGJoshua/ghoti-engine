#include "ECS/scene.h"
#include "ECS/component.h"
#include "ECS/system.h"

#include "data/hash_map.h"
#include "data/list.h"

#include "file/utilities.h"

#include "cJSON/cJSON.h"

#include <luajit-2.0/lauxlib.h>
#include <luajit-2.0/lualib.h>

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

Scene *createScene(void)
{
	Scene *ret = malloc(sizeof(Scene));

	ASSERT(ret != 0);

	ret->componentTypes = createHashMap(
		sizeof(UUID),
		sizeof(ComponentDataTable *),
		COMPONENT_TYPE_BUCKETS,
		(ComparisonOp)&strcmp);
	ret->entities = createHashMap(
		sizeof(UUID),
		sizeof(List),
		ENTITY_BUCKETS,
		(ComparisonOp)&strcmp);

	ret->physicsFrameSystems = createList(sizeof(System));
	ret->renderFrameSystems = createList(sizeof(System));
	ret->luaPhysicsFrameSystemNames = createList(sizeof(UUID));
	ret->luaRenderFrameSystemNames = createList(sizeof(UUID));

	return ret;
}

int32 loadScene(const char *name, Scene **scene)
{
	int32 error = 0;

	if (!scene)
	{
		return -1;
	}

	char *sceneFolder = getFolderPath(name, "resources/scenes");
	char *sceneFilename = getFullFilename(name, "scene", sceneFolder);
	free(sceneFolder);

	printf("Loading scene (%s)...\n", name);

	FILE *file = fopen(sceneFilename, "rb");

	if (file)
	{
		*scene = createScene();

		(*scene)->name = malloc(strlen(name) + 1);
		strcpy((*scene)->name, name);

		uint32 numSystemGroups;
		fread(&numSystemGroups, sizeof(uint32), 1, file);

		uint32 i, j;

		for (i = 0; i < numSystemGroups; i++)
		{
			char *systemGroup = readString(file);

			uint32 numExternalSystems;
			fread(&numExternalSystems, sizeof(uint32), 1, file);

			char **externalSystems = malloc(numExternalSystems * sizeof(char*));

			for (j = 0; j < numExternalSystems; j++)
			{
				externalSystems[j] = readString(file);
			}

			uint32 numInternalSystems;
			fread(&numInternalSystems, sizeof(uint32), 1, file);

			char **internalSystems = malloc(numInternalSystems * sizeof(char*));

			for (j = 0; j < numInternalSystems; j++)
			{
				internalSystems[j] = readString(file);
			}

			if (!strcmp(systemGroup, "update"))
			{
				for (j = 0; j < numExternalSystems; j++)
				{
					listPushBack(
						&(*scene)->luaPhysicsFrameSystemNames,
						&externalSystems[j]);
					// TODO: Internal system names
				}
			}
			else if (!strcmp(systemGroup, "draw"))
			{
				for (j = 0; j < numExternalSystems; j++)
				{
					listPushBack(
						&(*scene)->luaRenderFrameSystemNames,
						&externalSystems[j]);
					// TODO: Internal system names
				}
			}

			free(systemGroup);

			for (j = 0; j < numExternalSystems; j++)
			{
				free(externalSystems[j]);
			}

			free(externalSystems);

			for (j = 0; j < numInternalSystems; j++)
			{
				free(internalSystems[j]);
			}

			free(internalSystems);
		}

		uint32 numComponentLimits;
		fread(&numComponentLimits, sizeof(uint32), 1, file);

		char **componentLimitNames = malloc(numComponentLimits * sizeof(char*));
		uint32 *componentLimitNumbers = malloc(
			numComponentLimits * sizeof(uint32));

		for (i = 0; i < numComponentLimits; i++)
		{
			componentLimitNames[i] = readString(file);
			fread(&componentLimitNumbers[i], sizeof(uint32), 1, file);
		}

		loadSceneEntities(scene, name, false);

		for (i = 0; i < numComponentLimits; i++)
		{
			UUID componentName = idFromName(componentLimitNames[i]);
			sceneAddComponentType(
				*scene,
				componentName,
				getComponentDefinition(*scene, componentName).size,
				componentLimitNumbers[i]);
		}

		loadSceneEntities(scene, name, true);

		for (i = 0; i < numComponentLimits; i++)
		{
			free(componentLimitNames[i]);
		}

		free(componentLimitNames);
		free(componentLimitNumbers);

		UUID activeCamera;
		fread(activeCamera.bytes, UUID_LENGTH, 1, file);
		(*scene)->mainCamera = activeCamera;

		fclose(file);
	}
	else
	{
		printf("Failed to open %s\n", sceneFilename);
		error = -1;
	}

	free(sceneFilename);

	if (error != -1)
	{
		printf("Successfully loaded scene (%s)\n", name);
	}

	return error;
}

#define COMPONENT_DEFINITON_REALLOCATION_AMOUNT 32

int32 loadSceneEntities(Scene **scene, const char *name, bool loadData)
{
	int32 error = 0;

	if (!scene)
	{
		return -1;
	}

	char *sceneFolder = getFolderPath(name, "resources/scenes");
	char *entityFolder = getFullFilename("entities", NULL, sceneFolder);
	free(sceneFolder);

	DIR *dir = opendir(entityFolder);

	if (dir)
	{
		uint32 i;

		struct dirent *dirEntry = readdir(dir);

		if (!loadData)
		{
			(*scene)->numComponentsDefinitions = 0;
		}

		uint32 componentDefinitionsCapacity = 0;

		while (dirEntry)
		{
			char *a = strrchr(dirEntry->d_name, '.') + 1;
			char *b = dirEntry->d_name + strlen(dirEntry->d_name);
			char *extension = malloc(b - a + 1);

			memcpy(extension, a, b - a);
			extension[b - a] = '\0';

			if (strcmp(extension, "entity"))
			{
				free(extension);
				dirEntry = readdir(dir);
				continue;
			}

			free(extension);

			char *entityFilename = getFullFilename(
				dirEntry->d_name,
				NULL,
				entityFolder);

			FILE *file = fopen(entityFilename, "rb");

			if (file)
			{
				UUID uuid;
				fread(uuid.bytes, UUID_LENGTH + 1, 1, file);

				if (loadData)
				{
					sceneRegisterEntity(*scene, uuid);
				}

				uint32 numComponents;
				fread(&numComponents, sizeof(uint32), 1, file);

				if (!loadData)
				{
					if ((*scene)->numComponentsDefinitions + numComponents
						> componentDefinitionsCapacity)
					{
						while (
							(*scene)->numComponentsDefinitions + numComponents
							> componentDefinitionsCapacity)
						{
							componentDefinitionsCapacity +=
								COMPONENT_DEFINITON_REALLOCATION_AMOUNT;
						}

						uint32 previousBufferSize =
							(*scene)->numComponentsDefinitions
							* sizeof(ComponentDefinition);
						uint32 newBufferSize = componentDefinitionsCapacity
							* sizeof(ComponentDefinition);

						if (previousBufferSize == 0)
						{
							(*scene)->componentDefinitions =
								calloc(newBufferSize, 1);
						}
						else
						{
							(*scene)->componentDefinitions = realloc(
								(*scene)->componentDefinitions,
								newBufferSize);
							memset(
								(*scene)->componentDefinitions + previousBufferSize,
								0,
								newBufferSize - previousBufferSize);
						}
					}
				}

				for (i = 0; i < numComponents; i++)
				{
					ComponentDefinition *componentDefinition = calloc(
						1,
						sizeof(ComponentDefinition));

					if (!loadData)
					{
						freeComponentDefinition(componentDefinition);
						free(componentDefinition);

						componentDefinition =
							&(*scene)->componentDefinitions[
							(*scene)->numComponentsDefinitions++];
					}

					componentDefinition->name = readString(file);

					fread(
						&componentDefinition->numValues,
						sizeof(uint32),
						1,
						file);

					componentDefinition->values =
						calloc(componentDefinition->numValues,
							sizeof(ComponentValueDefinition));

					for (
						uint32 j = 0;
						j < componentDefinition->numValues;
						j++)
					{
						ComponentValueDefinition *componentValueDefinition =
							&componentDefinition->values[j];

						componentValueDefinition->name = readString(file);

						int8 dataType;
						fread(
							&dataType,
							sizeof(int8),
							1,
							file);
						componentValueDefinition->type = (DataType)dataType;

						if (componentValueDefinition->type == STRING)
						{
							fread(
								&componentValueDefinition->maxStringSize,
								sizeof(uint32),
								1,
								file);
						}

						fread(
							&componentValueDefinition->count,
							sizeof(uint32),
							1,
							file);
					}

					fread(
						&componentDefinition->size,
						sizeof(uint32),
						1,
						file);

					void *data = malloc(componentDefinition->size);
					fread(data, componentDefinition->size, 1, file);

					if (loadData)
					{
						sceneAddComponentToEntity(
							*scene,
							uuid,
							idFromName(componentDefinition->name),
							data);

						freeComponentDefinition(componentDefinition);
						free(componentDefinition);
					}

					free(data);
				}

				fclose(file);
			}
			else
			{
				printf("Failed to open %s\n", entityFilename);
				error = -1;
				break;
			}

			free(entityFilename);

			dirEntry = readdir(dir);
		}

		closedir(dir);

		if (!loadData)
		{
			uint32 numUniqueComponentDefinitions = 0;
			ComponentDefinition *uniqueComponentDefinitions = calloc(
				(*scene)->numComponentsDefinitions,
				sizeof(ComponentDefinition));

			for (i = 0; i < (*scene)->numComponentsDefinitions; i++)
			{
				ComponentDefinition *componentDefinition =
					&(*scene)->componentDefinitions[i];

				bool unique = true;
				for (uint32 j = 0; j < numUniqueComponentDefinitions; j++)
				{
					if (!strcmp(
						componentDefinition->name,
						uniqueComponentDefinitions[j].name))
					{
						unique = false;
						break;
					}
				}

				if (unique)
				{
					copyComponentDefinition(
						&uniqueComponentDefinitions[
							numUniqueComponentDefinitions++],
						componentDefinition);
				}

				freeComponentDefinition(componentDefinition);
			}

			free((*scene)->componentDefinitions);
			uniqueComponentDefinitions = realloc(
				uniqueComponentDefinitions,
				numUniqueComponentDefinitions * sizeof(ComponentDefinition));

			(*scene)->componentDefinitions = uniqueComponentDefinitions;
			(*scene)->numComponentsDefinitions = numUniqueComponentDefinitions;
		}
	}
	else
	{
		printf("Failed to open %s\n", entityFolder);
		error = -1;
	}

	free(entityFolder);

	return error;
}

void freeScene(Scene **scene)
{
	free((*scene)->name);

	listClear(&(*scene)->luaPhysicsFrameSystemNames);
	listClear(&(*scene)->luaRenderFrameSystemNames);

	for (ListIterator itr = listGetIterator(&(*scene)->physicsFrameSystems);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		freeSystem(LIST_ITERATOR_GET_ELEMENT(System, itr));
	}
	listClear(&(*scene)->physicsFrameSystems);

	for (ListIterator itr = listGetIterator(&(*scene)->renderFrameSystems);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		freeSystem(LIST_ITERATOR_GET_ELEMENT(System, itr));
	}
	listClear(&(*scene)->renderFrameSystems);

	for (HashMapIterator itr = hashMapGetIterator((*scene)->entities);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		sceneRemoveEntity(*scene, *(UUID *)hashMapIteratorGetKey(itr));
	}

	for (HashMapIterator itr = hashMapGetIterator((*scene)->componentTypes);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		sceneRemoveComponentType(*scene, *(UUID *)hashMapIteratorGetKey(itr));
	}

	freeHashMap(&(*scene)->entities);
	freeHashMap(&(*scene)->componentTypes);

	for (uint32 i = 0; i < (*scene)->numComponentsDefinitions; i++)
	{
		freeComponentDefinition(&(*scene)->componentDefinitions[i]);
	}

	free((*scene)->componentDefinitions);

	free(*scene);
	*scene = 0;
}

ComponentDefinition getComponentDefinition(
	const Scene *scene,
	UUID name)
{
	ComponentDefinition componentDefinition;
	memset(&componentDefinition, 0, sizeof(ComponentDefinition));

	for (uint32 i = 0; i < scene->numComponentsDefinitions; i++)
	{
		if (!strcmp(scene->componentDefinitions[i].name, name.string))
		{
			return scene->componentDefinitions[i];
		}
	}

	return componentDefinition;
}

void copyComponentDefinition(
	ComponentDefinition *dest,
	ComponentDefinition *src)
{
	dest->name = malloc(strlen(src->name) + 1);
	strcpy(dest->name, src->name);

	dest->size = src->size;
	dest->numValues = src->numValues;

	dest->values = malloc(src->numValues * sizeof(ComponentValueDefinition));
	for (uint32 i = 0; i < src->numValues; i++)
	{
		dest->values[i].name = malloc(strlen(src->values[i].name) + 1);
		strcpy(dest->values[i].name, src->values[i].name);

		dest->values[i].type = src->values[i].type;
		dest->values[i].maxStringSize = src->values[i].maxStringSize;
		dest->values[i].count = src->values[i].count;
	}
}

void freeComponentDefinition(ComponentDefinition *componentDefinition)
{
	free(componentDefinition->name);

	for (uint32 i = 0; i < componentDefinition->numValues; i++)
	{
		free(componentDefinition->values[i].name);
	}

	free(componentDefinition->values);
}

uint32 getDataTypeSize(DataType type)
{
	switch (type) {
		case UINT8:
		case INT8:
		case CHAR:
			return 1;
		case UINT16:
		case INT16:
			return 2;
		case UINT32:
		case INT32:
		case FLOAT32:
		case BOOL:
			return 4;
		case UINT64:
		case INT64:
		case FLOAT64:
			return 8;
		default:
			break;
	}

	return 0;
}

char* getDataTypeString(
	const ComponentValueDefinition *componentValueDefinition)
{
	char *dataTypeString = malloc(256);

	switch (componentValueDefinition->type) {
		case UINT8:
			strcpy(dataTypeString, "uint8");
			break;
		case UINT16:
			strcpy(dataTypeString, "uint16");
			break;
		case UINT32:
			strcpy(dataTypeString, "uint32");
			break;
		case UINT64:
			strcpy(dataTypeString, "uint64");
			break;
		case INT8:
			strcpy(dataTypeString, "int8");
			break;
		case INT16:
			strcpy(dataTypeString, "int16");
			break;
		case INT32:
			strcpy(dataTypeString, "int32");
			break;
		case INT64:
			strcpy(dataTypeString, "int64");
			break;
		case FLOAT32:
			strcpy(dataTypeString, "float32");
			break;
		case FLOAT64:
			strcpy(dataTypeString, "float64");
			break;
		case BOOL:
			strcpy(dataTypeString, "bool");
			break;
		case CHAR:
			strcpy(dataTypeString, "char");
			break;
		case STRING:
			sprintf(
				dataTypeString,
				"char(%d)",
				componentValueDefinition->maxStringSize - 1);
			break;
		default:
			break;
	}

	return dataTypeString;
}

void exportEntity(const Scene *scene, UUID entity, const char *filename)
{
	cJSON *json = cJSON_CreateObject();

	cJSON_AddStringToObject(json, "uuid", entity.string);
	cJSON *jsonComponents = cJSON_AddObjectToObject(json, "components");

	List *components = (List*)hashMapGetKey(scene->entities, entity.bytes);

	for (ListIterator itr = listGetIterator(components);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		UUID *componentUUID = LIST_ITERATOR_GET_ELEMENT(UUID, itr);
		cJSON *jsonComponent = cJSON_AddArrayToObject(
			jsonComponents,
			componentUUID->string);

		ComponentDataTable **componentDataTable =
			(ComponentDataTable**)hashMapGetKey(
				scene->componentTypes,
				componentUUID->bytes);

		void *componentData = cdtGet((*componentDataTable), entity);

		ComponentDefinition componentDefinition = getComponentDefinition(
			scene,
			*componentUUID);

		uint32 maxValueSize = 0;
		for (uint32 i = 0; i < componentDefinition.numValues; i++)
		{
			ComponentValueDefinition *componentValueDefinition =
				&componentDefinition.values[i];

			uint32 size = 0;
			if (componentValueDefinition->type == STRING)
			{
				size = componentValueDefinition->maxStringSize;
			}
			else
			{
				size = getDataTypeSize(componentValueDefinition->type);
			}

			uint32 paddingSize = size;
			if (componentValueDefinition->type == STRING)
			{
				paddingSize = 1;
			}

			if (paddingSize > maxValueSize)
			{
				maxValueSize = paddingSize;
			}
		}

		uint32 bytesWritten = 0;

		for (uint32 i = 0; i < componentDefinition.numValues; i++)
		{
			ComponentValueDefinition *componentValueDefinition =
				&componentDefinition.values[i];

			uint32 size = 0;
			if (componentValueDefinition->type == STRING)
			{
				size = componentValueDefinition->maxStringSize;
			}
			else
			{
				size = getDataTypeSize(componentValueDefinition->type);
			}

			uint32 paddingSize = size;
			if (componentValueDefinition->type == STRING)
			{
				paddingSize = 1;
			}

			cJSON *jsonComponentValue = cJSON_CreateObject();
			cJSON_AddStringToObject(
				jsonComponentValue,
				"name",
				componentValueDefinition->name);

			char *dataTypeString = getDataTypeString(componentValueDefinition);

			cJSON *jsonComponentValueData = NULL;
			if (componentValueDefinition->count > 1)
			{
				jsonComponentValueData =
					cJSON_AddArrayToObject(jsonComponentValue, dataTypeString);
			}

			for (uint32 j = 0; j < componentValueDefinition->count; j++)
			{
				uint32 padding = 0;
				if (bytesWritten > 0)
				{
					padding = bytesWritten % paddingSize;
					if (padding > 0)
					{
						padding = maxValueSize - padding;
					}
				}

				void *valueData = componentData + bytesWritten;

				uint8 uint8Data;
				uint16 uint16Data;
				uint32 uint32Data;
				uint64 uint64Data;
				int8 int8Data;
				int16 int16Data;
				int32 int32Data;
				int64 int64Data;
				real32 real32Data;
				real64 real64Data;
				bool boolData;
				char charData[2];

				if (componentValueDefinition->count == 1)
				{
					switch (componentValueDefinition->type) {
						case UINT8:
							uint8Data = *(uint8*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)uint8Data);
							break;
						case UINT16:
							uint16Data = *(uint16*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)uint16Data);
							break;
						case UINT32:
							uint32Data = *(uint32*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)uint32Data);
							break;
						case UINT64:
							uint64Data = *(uint64*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)uint64Data);
							break;
						case INT8:
							int8Data = *(int8*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)int8Data);
							break;
						case INT16:
							int16Data = *(int16*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)int16Data);
							break;
						case INT32:
							int32Data = *(int32*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)int32Data);
							break;
						case INT64:
							int64Data = *(int64*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)int64Data);
							break;
						case FLOAT32:
							real32Data = *(real32*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)real32Data);
							break;
						case FLOAT64:
							real64Data = *(real64*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)real64Data);
							break;
						case BOOL:
							boolData = *(bool*)valueData;
							cJSON_AddBoolToObject(
								jsonComponentValue,
								dataTypeString,
								(cJSON_bool)boolData);
							break;
						case CHAR:
							charData[0] = *(char*)valueData;
							charData[1] = '\0';
							cJSON_AddStringToObject(
								jsonComponentValue,
								dataTypeString,
								charData);
							break;
						case STRING:
							cJSON_AddStringToObject(
								jsonComponentValue,
								dataTypeString,
								(char*)valueData);
							break;
						default:
							break;
					}
				}
				else
				{
					cJSON *jsonComponentValueDataArrayItem = NULL;

					switch (componentValueDefinition->type) {
						case UINT8:
							uint8Data = *(uint8*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)uint8Data);
							break;
						case UINT16:
							uint16Data = *(uint16*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)uint16Data);
							break;
						case UINT32:
							uint32Data = *(uint32*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)uint32Data);
							break;
						case UINT64:
							uint64Data = *(uint64*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)uint64Data);
							break;
						case INT8:
							int8Data = *(int8*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)int8Data);
							break;
						case INT16:
							int16Data = *(int16*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)int16Data);
							break;
						case INT32:
							int32Data = *(int32*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)int32Data);
							break;
						case INT64:
							int64Data = *(int64*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)int64Data);
							break;
						case FLOAT32:
							real32Data = *(real32*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)real32Data);
							break;
						case FLOAT64:
							real64Data = *(real64*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)real64Data);
							break;
						case BOOL:
							boolData = *(bool*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateBool((cJSON_bool)boolData);
							break;
						case CHAR:
							charData[0] = *(char*)valueData;
							charData[1] = '\0';
							jsonComponentValueDataArrayItem =
								cJSON_CreateString(charData);
							break;
						case STRING:
							jsonComponentValueDataArrayItem =
								cJSON_CreateString((char*)valueData);
							break;
						default:
							break;
					}

					cJSON_AddItemToArray(
						jsonComponentValueData,
						jsonComponentValueDataArrayItem);
				}

				bytesWritten += size + padding;
			}

			free(dataTypeString);
			cJSON_AddItemToArray(jsonComponent, jsonComponentValue);
		}
	}

	writeJSON(json, filename);
	cJSON_Delete(json);
}

// TODO: Export scenes to live representation when being unloaded

internal
char** getSystemNames(List *list, uint32 *numSystemNames)
{
	*numSystemNames = listGetSize(list);
	char **systemNames = malloc(*numSystemNames * sizeof(char*));

	uint32 i = 0;
	for (ListIterator itr = listGetIterator(list);
		!listIteratorAtEnd(itr);
		listMoveIterator(&itr))
	{
		char *systemName = *LIST_ITERATOR_GET_ELEMENT(char*, itr);
		systemNames[i++] = malloc(strlen(systemName) + 1);
		strcpy(systemNames[i++], systemName);
	}

	return systemNames;
}

internal
void freeSystemNames(char **systemNames, uint32 numSystemNames)
{
	for (uint32 i = 0; i < numSystemNames; i++)
	{
		free(systemNames[i]);
	}

	free(systemNames);
}

void exportScene(const Scene *scene, const char *filename)
{
	printf("Exporting scene (%s)...\n", scene->name);

	cJSON *json = cJSON_CreateObject();

	cJSON *systems = cJSON_AddObjectToObject(json, "systems");

	cJSON *updateSystems = cJSON_AddObjectToObject(systems, "update");

	uint32 numSystemNames;
	char **systemNames = getSystemNames(
		(List*)&scene->luaPhysicsFrameSystemNames,
		&numSystemNames);

	cJSON *externalUpdateSystems = cJSON_CreateStringArray(
		(const char**)systemNames,
		numSystemNames);
	freeSystemNames(systemNames, numSystemNames);

	cJSON_AddItemToObject(updateSystems, "external", externalUpdateSystems);

	// TODO: Internal system names
	// systemNames = getSystemNames(
	// 	(List*)&scene->physicsFrameSystemNames,
	// 	&numSystemNames);

	// cJSON *internalUpdateSystems = cJSON_CreateStringArray(
	// 	(const char**)systemNames,
	// 	numSystemNames);
	// freeSystemNames(systemNames, numSystemNames);

	// cJSON_AddItemToObject(updateSystems, "internal", internalUpdateSystems);

	cJSON *drawSystems = cJSON_AddObjectToObject(systems, "draw");

	systemNames = getSystemNames(
		(List*)&scene->luaRenderFrameSystemNames,
		&numSystemNames);

	cJSON *externalDrawSystems = cJSON_CreateStringArray(
		(const char**)systemNames,
		numSystemNames);
	freeSystemNames(systemNames, numSystemNames);

	cJSON_AddItemToObject(drawSystems, "external", externalDrawSystems);

	// TODO: Internal system names
	// systemNames = getSystemNames(
	// 	(List*)&scene->renderFrameSystemNames,
	// 	&numSystemNames);

	// cJSON *internalDrawSystems = cJSON_CreateStringArray(
	// 	(const char**)systemNames,
	// 	numSystemNames);
	// freeSystemNames(systemNames, numSystemNames);

	// cJSON_AddItemToObject(drawSystems, "internal", internalDrawSystems);

	cJSON *componentLimits = cJSON_AddObjectToObject(json, "component_limits");

	for (HashMapIterator itr = hashMapGetIterator(scene->componentTypes);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		UUID *componentUUID = (UUID*)hashMapIteratorGetKey(itr);
		uint32 componentLimit =
			((ComponentDataTable*)hashMapIteratorGetValue(itr))->numEntries;
		cJSON_AddNumberToObject(
			componentLimits,
			componentUUID->string,
			componentLimit);
	}

	cJSON_AddStringToObject(json, "active_camera", scene->mainCamera.string);

	writeJSON(json, filename);
	cJSON_Delete(json);

	printf("Successfully exported scene (%s)\n", scene->name);
}

void exportSave(void *data, uint32 size, const Scene *scene, uint32 slot)
{
	char *saveName = malloc(128);
	sprintf(saveName, "save_%d", slot);

	printf("Exporting save file (%s)...\n", saveName);

	char *saveFolder = malloc(512);
	sprintf(saveFolder, "resources/saves/%s", saveName);
	char *saveFilename = getFullFilename(saveName, "save", saveFolder);

	MKDIR(saveFolder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	char *sceneFolder = getFolderPath(scene->name, saveFolder);
	char *sceneFilename = getFullFilename(scene->name, NULL, sceneFolder);
	char *entitiesFolder = getFullFilename("entities", NULL, sceneFolder);

	MKDIR(sceneFolder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	MKDIR(entitiesFolder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	FILE *file = fopen(saveFilename, "wb");

	writeString(scene->name, file);
	fwrite(&size, sizeof(uint32), 1, file);
	fwrite(data, size, 1, file);

	fclose(file);

	exportScene(scene, sceneFilename);

	uint32 entityNumber = 0;
	for (HashMapIterator itr = hashMapGetIterator(scene->entities);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		char *entityName = malloc(128);
		sprintf(entityName, "entity_%d", entityNumber++);
		char *entityFilename = getFullFilename(
			entityName,
			NULL,
			entitiesFolder);
		free(entityName);

		UUID *entity = (UUID*)hashMapIteratorGetKey(itr);
		exportEntity(scene, *entity, entityFilename);
		free(entityFilename);
	}

	free(saveFolder);
	free(saveFilename);
	free(sceneFolder);
	free(sceneFilename);
	free(entitiesFolder);

	printf("Successfully exported save file (%s)\n", saveName);
	free(saveName);
}

inline
void sceneAddRenderFrameSystem(
	Scene *scene,
	System system)
{
	listPushBack(&scene->renderFrameSystems, &system);
}

inline
void sceneAddPhysicsFrameSystem(
	Scene *scene,
	System system)
{
	listPushBack(&scene->physicsFrameSystems, &system);
}

void sceneInitRenderFrameSystems(Scene *scene)
{
	ListIterator itr = listGetIterator(&scene->renderFrameSystems);
	while (!listIteratorAtEnd(itr))
	{
		if (LIST_ITERATOR_GET_ELEMENT(System, itr)->init != 0)
		{
			LIST_ITERATOR_GET_ELEMENT(System, itr)->init(scene);
		}

		listMoveIterator(&itr);
	}
}

void sceneInitPhysicsFrameSystems(Scene *scene)
{
	ListIterator itr = listGetIterator(&scene->physicsFrameSystems);
	while (!listIteratorAtEnd(itr))
	{
		if (LIST_ITERATOR_GET_ELEMENT(System, itr)->init != 0)
		{
			LIST_ITERATOR_GET_ELEMENT(System, itr)->init(scene);
		}

		listMoveIterator(&itr);
	}
}

void sceneInitSystems(Scene *scene)
{
	sceneInitRenderFrameSystems(scene);
	sceneInitPhysicsFrameSystems(scene);
}

void sceneRunRenderFrameSystems(Scene *scene, real64 dt)
{
	ListIterator itr = listGetIterator(&scene->renderFrameSystems);
	while (!listIteratorAtEnd(itr))
	{
		systemRun(scene, LIST_ITERATOR_GET_ELEMENT(System, itr), dt);

		listMoveIterator(&itr);
	}
}

void sceneRunPhysicsFrameSystems(Scene *scene, real64 dt)
{
	ListIterator itr = listGetIterator(&scene->physicsFrameSystems);
	while (!listIteratorAtEnd(itr))
	{
		systemRun(scene, LIST_ITERATOR_GET_ELEMENT(System, itr), dt);

		listMoveIterator(&itr);
	}
}

void sceneShutdownRenderFrameSystems(Scene *scene)
{
	ListIterator itr = listGetIterator(&scene->renderFrameSystems);
	while (!listIteratorAtEnd(itr))
	{
		if (LIST_ITERATOR_GET_ELEMENT(System, itr)->shutdown != 0)
		{
			LIST_ITERATOR_GET_ELEMENT(System, itr)->shutdown(scene);
		}

		listMoveIterator(&itr);
	}
}

void sceneShutdownPhysicsFrameSystems(Scene *scene)
{
	ListIterator itr = listGetIterator(&scene->physicsFrameSystems);
	while (!listIteratorAtEnd(itr))
	{
		if (LIST_ITERATOR_GET_ELEMENT(System, itr)->shutdown != 0)
		{
			LIST_ITERATOR_GET_ELEMENT(System, itr)->shutdown(scene);
		}

		listMoveIterator(&itr);
	}
}

void sceneShutdownSystems(Scene *scene)
{
	sceneShutdownRenderFrameSystems(scene);
	sceneShutdownPhysicsFrameSystems(scene);
}

void sceneInitLua(lua_State **L, Scene *scene)
{
	lua_getglobal(*L, "engine");
	lua_getfield(*L, -1, "initScene");
	lua_remove(*L, -2);
	lua_pushlightuserdata(*L, scene);
	int luaError = lua_pcall(*L, 1, 0, 0);
	if (luaError)
	{
		printf("Lua error: %s\n", lua_tostring(*L, -1));
		lua_pop(*L, 1);
		lua_close(*L);
	}
}

void sceneShutdownLua(lua_State **L, Scene *scene)
{
	lua_getglobal(*L, "engine");
	lua_getfield(*L, -1, "shutdownScene");
	lua_remove(*L, -2);
	lua_pushlightuserdata(*L, scene);
	int luaError = lua_pcall(*L, 1, 0, 0);
	if (luaError)
	{
		printf("Lua error: %s\n", lua_tostring(*L, -1));
		lua_pop(*L, 1);
		lua_close(*L);
	}
}

void sceneAddComponentType(
	Scene *scene,
	UUID componentID,
	uint32 componentSize,
	uint32 maxComponents)
{
	ComponentDataTable *table = createComponentDataTable(
		maxComponents,
		componentSize);

	ASSERT(table);

	hashMapInsert(scene->componentTypes, &componentID, &table);

	ASSERT(hashMapGetKey(scene->componentTypes, &componentID));
	ASSERT(table == *(ComponentDataTable **)
		hashMapGetKey(
			scene->componentTypes,
			&componentID));
}

void sceneRemoveComponentType(Scene *scene, UUID componentID)
{
	// Iterate over every entity
	HashMapIterator itr = hashMapGetIterator(scene->entities);
	while (!hashMapIteratorAtEnd(itr))
	{
		// Iterate over every component type in the entity
		ListIterator litr = listGetIterator((List *)hashMapIteratorGetValue(itr));
		while (!listIteratorAtEnd(litr))
		{
			if (!strcmp(LIST_ITERATOR_GET_ELEMENT(UUID, litr)->string, componentID.string))
			{
				listRemove((List *)hashMapIteratorGetValue(itr), litr);
			}
			listMoveIterator(&litr);
		}

		hashMapMoveIterator(&itr);
	}

	ComponentDataTable **temp = (ComponentDataTable **)hashMapGetKey(
		scene->componentTypes,
		&componentID);

	if (temp)
	{
		freeComponentDataTable(temp);
	}
}

internal
UUID generateUUID()
{
	UUID ret;

	// Generate random byte for all but the last byte
	for (uint32 i = 0; i < sizeof(UUID) - 1; ++i)
	{
		do
		{
			ret.bytes[i] = (rand() % (126 - 35)) + 35;
		} while (ret.bytes[i] == 92);
	}

	ret.bytes[sizeof(UUID) - 1] = 0;

	return ret;
}

void sceneRegisterEntity(Scene *s, UUID newEntity)
{
	List emptyList = createList(sizeof(UUID));
	hashMapInsert(s->entities, &newEntity, &emptyList);
}

UUID sceneCreateEntity(Scene *s)
{
	UUID newEntity = generateUUID();
	sceneRegisterEntity(s, newEntity);
	return newEntity;
}

void sceneRemoveEntity(Scene *s, UUID entity)
{
	List *entityComponentList = hashMapGetKey(s->entities, &entity);

	if (!entityComponentList)
	{
		return;
	}

	// For each component type
	for (ListIterator listIterator = listGetIterator(entityComponentList);
		 !listIteratorAtEnd(listIterator);
		 listMoveIterator(&listIterator))
	{
		ComponentDataTable **table = hashMapGetKey(
			s->componentTypes,
			LIST_ITERATOR_GET_ELEMENT(void, listIterator));

		if (!table || !*table)
		{
			continue;
		}

		// Remove this entity from the components
		cdtRemove(*table, entity);
	}

	// Clear component list
	listClear(entityComponentList);
	hashMapDeleteKey(s->entities, &entity);
}

void sceneAddComponentToEntity(
	Scene *s,
	UUID entity,
	UUID componentType,
	void *componentData)
{
	printf("Adding %s component to entity with id %s\n", componentType.string, entity.string);

	// Get the data table
	ComponentDataTable **dataTable = hashMapGetKey(
		s->componentTypes,
		&componentType);

	if (!dataTable || !*dataTable)
	{
		return;
	}

	// Add the component to the data table
	cdtInsert(
		*dataTable,
		entity,
		componentData);
	// Add the component type to the list
	listPushBack(hashMapGetKey(s->entities, &entity), &componentType);
}

void sceneRemoveComponentFromEntity(
	Scene *s,
	UUID entity,
	UUID componentType)
{
	ComponentDataTable **table = hashMapGetKey(
		s->componentTypes,
		&componentType);

	if (!table || !*table)
	{
		return;
	}

	cdtRemove(*table, entity);

	List *componentTypeList = hashMapGetKey(s->entities, &entity);

	if (!componentTypeList)
	{
		return;
	}

	for (ListIterator itr = listGetIterator(componentTypeList);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		if (strcmp(LIST_ITERATOR_GET_ELEMENT(UUID, itr)->string, componentType.string))
		{
			listRemove(componentTypeList, itr);
		}
	}
}

void *sceneGetComponentFromEntity(
	Scene *s,
	UUID entity,
	UUID componentType)
{
	ComponentDataTable **table = hashMapGetKey(
		s->componentTypes,
		&componentType);

	if (!table || !*table)
	{
		return 0;
	}

	return cdtGet(*table, entity);
}

inline
UUID idFromName(const char *name)
{
	UUID ret = {};
	strcpy(ret.string, name);
	return ret;
}
