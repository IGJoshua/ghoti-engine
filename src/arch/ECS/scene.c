#include "ECS/scene.h"
#include "ECS/component.h"
#include "ECS/system.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "components/component_types.h"
#include "components/rigid_body.h"

#include "asset_management/model.h"

#include "file/utilities.h"

#include <cjson/cJSON.h>

#include "json-utilities/utilities.h"

#include <luajit-2.0/lauxlib.h>
#include <luajit-2.0/lualib.h>

#include <ode/ode.h>

#include <sys/stat.h>

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>

#define MAX_CONTACTS 4096

extern HashMap systemRegistry;
extern List activeScenes;
extern bool changeScene;
extern bool reloadingScene;
extern List unloadedScenes;

internal ComponentDefinition getComponentDefinition(
	Scene *scene,
	UUID name);
internal void freeComponentDefinition(ComponentDefinition *componentDefinition);

internal uint32 getDataTypeSize(DataType type);
internal char* getDataTypeString(
	const ComponentValueDefinition *componentValueDefinition);

Scene *createScene(void)
{
	Scene *ret = calloc(1, sizeof(Scene));

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

	ret->physicsFrameSystems = createList(sizeof(UUID));
	ret->renderFrameSystems = createList(sizeof(UUID));
	ret->luaPhysicsFrameSystemNames = createList(sizeof(UUID));
	ret->luaRenderFrameSystemNames = createList(sizeof(UUID));

	ret->physicsWorld = dWorldCreate();
	ret->physicsSpace = dHashSpaceCreate(0);
	ret->contactGroup = dJointGroupCreate(MAX_CONTACTS);

	dWorldSetGravity(ret->physicsWorld, 0, -9.8f, 0);
	dWorldSetAutoDisableFlag(ret->physicsWorld, 1);

	return ret;
}

internal
int32 exportSceneJSONEntities(const char *folder)
{
	int32 error = 0;

	DIR *dir = opendir(folder);

	if (dir)
	{
		struct dirent *dirEntry = readdir(dir);
		while (dirEntry)
		{
			if (strcmp(dirEntry->d_name, ".") && strcmp(dirEntry->d_name, ".."))
			{
				char *folderPath = getFullFilePath(
					dirEntry->d_name,
					NULL,
					folder);

				struct stat info;
				stat(folderPath, &info);

				if (S_ISDIR(info.st_mode))
				{
					if (exportSceneJSONEntities(folderPath) == -1)
					{
						free(folderPath);
						closedir(dir);
						return -1;
					}
				}
				else if (S_ISREG(info.st_mode))
				{
					char *extension = getExtension(dirEntry->d_name);
					if (extension && !strcmp(extension, "json"))
					{
						char *entityFilename = removeExtension(
							dirEntry->d_name);
						char *jsonEntityFilename = getFullFilePath(
							entityFilename,
							NULL,
							folder);
						free(entityFilename);

						if (exportEntity(
							jsonEntityFilename,
							LOG_FILE_NAME) == -1)
						{
							LOG("Failed to export entity\n");
							free(jsonEntityFilename);
							free(folderPath);
							free(extension);
							closedir(dir);
							return -1;
						}

						free(jsonEntityFilename);
					}

					free(extension);
				}

				free(folderPath);
			}

			dirEntry = readdir(dir);
		}

		closedir(dir);
	}
	else
	{
		LOG("Failed to open %s\n", folder);
		error = -1;
	}

	return error;
}

internal
int32 loadSceneEntities(
	Scene **scene,
	bool loadData,
	bool recursive,
	const char *folder)
{
	int32 error = 0;

	if (!scene)
	{
		return -1;
	}

	DIR *dir = opendir(folder);

	if (dir)
	{
		uint32 i;

		struct dirent *dirEntry = readdir(dir);

		if (!loadData && !recursive)
		{
			(*scene)->componentDefinitions = createHashMap(
				sizeof(UUID),
				sizeof(ComponentDefinition),
				COMPONENT_DEFINITION_BUCKETS,
				(ComparisonOp)&strcmp);
		}

		while (dirEntry)
		{
			if (strcmp(dirEntry->d_name, ".") && strcmp(dirEntry->d_name, ".."))
			{
				char *folderPath = getFullFilePath(
					dirEntry->d_name,
					NULL,
					folder);

				struct stat info;
				stat(folderPath, &info);

				if (S_ISDIR(info.st_mode))
				{
					if (loadSceneEntities(
						scene,
						loadData,
						true,
						folderPath) == -1)
					{
						LOG("Failed to load scene entities\n");
						free(folderPath);
						closedir(dir);
						return -1;
					}
				}
				else if (S_ISREG(info.st_mode))
				{
					char *extension = getExtension(dirEntry->d_name);
					if (extension && !strcmp(extension, "entity"))
					{
						FILE *file = fopen(folderPath, "rb");

						if (file)
						{
							UUID uuid = readUUID(file);

							if (loadData)
							{
								sceneRegisterEntity(*scene, uuid);
							}

							uint32 numComponents;
							fread(&numComponents, sizeof(uint32), 1, file);

							for (i = 0; i < numComponents; i++)
							{
								ComponentDefinition *componentDefinition =
									calloc(1, sizeof(ComponentDefinition));

								componentDefinition->name = readString(file);

								fread(&componentDefinition->numValues,
									sizeof(uint32),
									1,
									file);

								componentDefinition->values =
									calloc(componentDefinition->numValues,
										sizeof(ComponentValueDefinition));

								for (uint32 j = 0;
									j < componentDefinition->numValues;
									j++)
								{
									ComponentValueDefinition
										*componentValueDefinition =
											&componentDefinition->values[j];

									componentValueDefinition->name =
										readString(file);

									int8 dataType;
									fread(
										&dataType,
										sizeof(int8),
										1,
										file);
									componentValueDefinition->type =
										(DataType)dataType;

									if (componentValueDefinition->type
										== DATA_TYPE_STRING)
									{
										fread(
											&componentValueDefinition
												->maxStringSize,
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

								if (!loadData)
								{
									UUID componentID =
										idFromName(componentDefinition->name);

									ComponentDefinition
										*existingComponentDefinition =
										(ComponentDefinition*)
											hashMapGetData(
												(*scene)->componentDefinitions,
												&componentID);

									if (!existingComponentDefinition)
									{
										hashMapInsert(
											(*scene)->componentDefinitions,
											&componentID,
											componentDefinition);
									}
									else
									{
										freeComponentDefinition(
											componentDefinition);
									}
								}
								else
								{
									sceneAddComponentToEntity(
										*scene,
										uuid,
										idFromName(componentDefinition->name),
										data);

									freeComponentDefinition(
										componentDefinition);
								}

								free(componentDefinition);
								free(data);
							}

							fclose(file);
						}
						else
						{
							LOG("Failed to open %s\n", folderPath);
							free(folderPath);
							free(extension);
							closedir(dir);
							return -1;
						}
					}

					free(extension);
				}

				free(folderPath);
			}

			dirEntry = readdir(dir);
		}

		closedir(dir);
	}
	else
	{
		LOG("Failed to open %s\n", folder);
		error = -1;
	}

	return error;
}

int32 loadSceneFile(const char *name, Scene **scene)
{
	int32 error = 0;

	if (!scene)
	{
		return -1;
	}

	LOG("Loading scene (%s)...\n", name);

	char *sceneFolder = NULL;

	bool found = false;

	if (!reloadingScene)
	{
		DIR *dir = opendir(RUNTIME_STATE_DIR);
		if (dir)
		{
			struct dirent *dirEntry = readdir(dir);
			while (dirEntry)
			{
				if (!strcmp(dirEntry->d_name, name))
				{
					found = true;
					break;
				}

				dirEntry = readdir(dir);
			}

			closedir(dir);
		}
	}

	if (found)
	{
		sceneFolder = getFullFilePath(
			name,
			NULL,
			RUNTIME_STATE_DIR);
	}
	else
	{
		sceneFolder = getFullFilePath(
			name,
			NULL,
			"resources/scenes");
	}

	char *sceneFilename = getFullFilePath(name, NULL, sceneFolder);
	char *jsonSceneFilename = getFullFilePath(
		sceneFilename,
		"json",
		NULL);

	if (access(jsonSceneFilename, F_OK) != -1)
	{
		if (exportScene(sceneFilename, LOG_FILE_NAME) == -1)
		{
			LOG("Failed to export scene.\n");
			free(jsonSceneFilename);
			free(sceneFilename);
			free(sceneFolder);
			return -1;
		}
	}

	free(jsonSceneFilename);
	free(sceneFilename);

	sceneFilename = getFullFilePath(name, "scene", sceneFolder);

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

			UUID systemName = {};

			if (!strcmp(systemGroup, "update"))
			{
				for (j = 0; j < numExternalSystems; j++)
				{
					strcpy(systemName.string, externalSystems[j]);
					listPushBack(
						&(*scene)->luaPhysicsFrameSystemNames,
						&systemName);
				}

				for (j = 0; j < numInternalSystems; j++)
				{
					strcpy(systemName.string, internalSystems[j]);
					sceneAddPhysicsFrameSystem(*scene, systemName);
				}
			}
			else if (!strcmp(systemGroup, "draw"))
			{
				for (j = 0; j < numExternalSystems; j++)
				{
					strcpy(systemName.string, externalSystems[j]);
					listPushBack(
						&(*scene)->luaRenderFrameSystemNames,
						&systemName);
				}

				for (j = 0; j < numInternalSystems; j++)
				{
					strcpy(systemName.string, internalSystems[j]);
					sceneAddRenderFrameSystem(*scene, systemName);
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

		char *entityFolder = getFullFilePath("entities", NULL, sceneFolder);
		if (exportSceneJSONEntities(entityFolder) == -1)
		{
			free(sceneFilename);
			free(entityFolder);
			free(sceneFolder);
			free(componentLimitNumbers);
			fclose(file);

			reloadingScene = true;
			freeScene(scene);
			reloadingScene = false;

			return -1;
		}

		if (loadSceneEntities(scene, false, false, entityFolder) == -1)
		{
			LOG("Failed to load scene entities\n");
			free(sceneFilename);
			free(entityFolder);
			free(sceneFolder);
			free(componentLimitNumbers);
			fclose(file);

			reloadingScene = true;
			freeScene(scene);
			reloadingScene = false;

			return -1;
		}

		for (i = 0; i < numComponentLimits; i++)
		{
			UUID componentName = idFromName(componentLimitNames[i]);
			sceneAddComponentType(
				*scene,
				componentName,
				getComponentDefinition(*scene, componentName).size,
				componentLimitNumbers[i]);
		}

		(*scene)->numComponentLimitNames = numComponentLimits;
		(*scene)->componentLimitNames = componentLimitNames;

		if (loadSceneEntities(scene, true, false, entityFolder) == -1)
		{
			LOG("Failed to load scene entities\n");
			free(sceneFilename);
			free(entityFolder);
			free(sceneFolder);
			free(componentLimitNumbers);
			fclose(file);

			reloadingScene = true;
			freeScene(scene);
			reloadingScene = false;

			return -1;
		}

		free(entityFolder);
		free(componentLimitNumbers);

		(*scene)->mainCamera = readUUID(file);

		fread(&(*scene)->gravity, sizeof(real32), 1, file);

		fclose(file);
	}
	else
	{
		LOG("Failed to open %s\n", sceneFilename);
		error = -1;
	}

	free(sceneFolder);
	free(sceneFilename);

	if (error != -1)
	{
		for (HashMapIterator itr =
				hashMapGetIterator((*scene)->componentDefinitions);
			!hashMapIteratorAtEnd(itr);
			hashMapMoveIterator(&itr))
		{
			ComponentDefinition *componentDefintion =
				(ComponentDefinition*)hashMapIteratorGetValue(itr);

			bool found = false;
			for (uint32 i = 0; i < (*scene)->numComponentLimitNames; i++)
			{
				char *componentLimitName = (*scene)->componentLimitNames[i];
				if (!strcmp(componentDefintion->name, componentLimitName))
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				LOG("ERROR: Component limit for the %s component "
					"is missing from the scene\n",
					componentDefintion->name);
				ASSERT(false);
			}
		}

		LOG("Successfully loaded scene (%s)\n", name);
	}

	return error;
}

Scene *getScene(const char *name)
{
	for (ListIterator itr = listGetIterator(&activeScenes);
		!listIteratorAtEnd(itr);
		listMoveIterator(&itr))
	{
		Scene *scene = *LIST_ITERATOR_GET_ELEMENT(Scene*, itr);
		if (!strcmp(name, scene->name))
		{
			return scene;
		}
	}

	return NULL;
}

internal
void exportRuntimeScene(Scene *scene)
{
	MKDIR(RUNTIME_STATE_DIR);

	char *sceneFolder = getFullFilePath(
		scene->name,
		NULL,
		RUNTIME_STATE_DIR);

	deleteFolder(sceneFolder, false);

	char *sceneFilename = getFullFilePath(
		scene->name,
		NULL,
		sceneFolder);
	char *entitiesFolder = getFullFilePath("entities", NULL, sceneFolder);

	MKDIR(sceneFolder);
	MKDIR(entitiesFolder);

	exportSceneSnapshot(scene, sceneFilename);

	if (exportScene(sceneFilename, LOG_FILE_NAME) == -1)
	{
		free(sceneFolder);
		free(sceneFilename);
		free(entitiesFolder);
		return;
	}

	char *jsonSceneFilename = getFullFilePath(
		sceneFilename,
		"json",
		NULL);
	remove(jsonSceneFilename);
	free(jsonSceneFilename);

	uint32 entityNumber = 0;
	for (HashMapIterator itr = hashMapGetIterator(scene->entities);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		char *entityName = malloc(128);
		sprintf(entityName, "entity_%d", entityNumber++);
		char *entityFilename = getFullFilePath(
			entityName,
			NULL,
			entitiesFolder);
		free(entityName);

		UUID *entity = (UUID*)hashMapIteratorGetKey(itr);
		exportEntitySnapshot(scene, *entity, entityFilename);

		if (exportEntity(entityFilename, LOG_FILE_NAME) == -1)
		{
			free(entityFilename);
			free(sceneFolder);
			free(sceneFilename);
			free(entitiesFolder);
			return;
		}

		char *jsonEntityFilename = getFullFilePath(
			entityFilename,
			"json",
			NULL);
		remove(jsonEntityFilename);

		free(jsonEntityFilename);
		free(entityFilename);
	}

	free(sceneFolder);
	free(sceneFilename);
	free(entitiesFolder);
}

void freeScene(Scene **scene)
{
	LOG("Unloading scene (%s)...\n", (*scene)->name);

	if (!reloadingScene)
	{
		exportRuntimeScene(*scene);
	}

	listClear(&(*scene)->luaPhysicsFrameSystemNames);
	listClear(&(*scene)->luaRenderFrameSystemNames);
	listClear(&(*scene)->physicsFrameSystems);
	listClear(&(*scene)->renderFrameSystems);

	if ((*scene)->entities) {
		for (HashMapIterator itr = hashMapGetIterator((*scene)->entities);
			!hashMapIteratorAtEnd(itr);
			hashMapMoveIterator(&itr))
		{
			UUID entity = *(UUID*)hashMapIteratorGetKey(itr);
			sceneRemoveEntityComponents(*scene, entity);
		}
	}

	if ((*scene)->componentTypes) {
		for (HashMapIterator itr =
			hashMapGetIterator((*scene)->componentTypes);
			!hashMapIteratorAtEnd(itr);
			hashMapMoveIterator(&itr))
		{
			sceneRemoveComponentType(
				*scene,
				*(UUID *)hashMapIteratorGetKey(itr));
		}
	}

	if ((*scene)->componentDefinitions) {
		for (HashMapIterator itr =
				hashMapGetIterator((*scene)->componentDefinitions);
			!hashMapIteratorAtEnd(itr);
			hashMapMoveIterator(&itr))
		{
			freeComponentDefinition(
				(ComponentDefinition*)hashMapIteratorGetValue(itr));
		}
	}

	if ((*scene)->entities) {
		freeHashMap(&(*scene)->entities);
	}

	if ((*scene)->componentTypes) {
		freeHashMap(&(*scene)->componentTypes);
	}

	if ((*scene)->componentDefinitions) {
		freeHashMap(&(*scene)->componentDefinitions);
	}

	for (uint32 i = 0; i < (*scene)->numComponentLimitNames; i++)
	{
		free((*scene)->componentLimitNames[i]);
	}

	free((*scene)->componentLimitNames);

	dJointGroupDestroy((*scene)->contactGroup);
	dSpaceDestroy((*scene)->physicsSpace);
	dWorldDestroy((*scene)->physicsWorld);

	LOG("Successfully unloaded scene (%s)\n", (*scene)->name);
	free((*scene)->name);

	free(*scene);
	*scene = 0;
}

extern lua_State *L;

int32 loadScene(const char *name)
{
	if (!getScene(name))
	{
		for (ListIterator itr = listGetIterator(&unloadedScenes);
			 !listIteratorAtEnd(itr);
			 listMoveIterator(&itr))
		{
			Scene *scene = *LIST_ITERATOR_GET_ELEMENT(Scene*, itr);
			if (!strcmp(name, scene->name))
			{
				listRemove(&unloadedScenes, &itr);
				listPushFront(&activeScenes, &scene);

				scene->loadedThisFrame = true;

				return 0;
			}
		}

		Scene *scene;
		if (loadSceneFile(name, &scene) == -1)
		{
			LOG("Failed to load scene (%s)\n", name);
			return -1;
		}

		sceneInitSystems(scene);
		sceneInitLua(&L, scene);

		listPushFront(&activeScenes, &scene);

		scene->loadedThisFrame = true;

		dWorldSetGravity(scene->physicsWorld, 0, scene->gravity, 0);

		return 0;
	}

	return -1;
}

int32 reloadScene(const char *name)
{
	int32 error = unloadScene(name);

	if (!error)
	{
		reloadingScene = true;
		return 0;
	}

	return error;
}

int32 reloadAllScenes(void)
{
	int32 error = 0;

	for (ListIterator itr = listGetIterator(&activeScenes);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		error = reloadScene((*LIST_ITERATOR_GET_ELEMENT(Scene*, itr))->name);

		if (error)
		{
			break;
		}
	}

	return error;
}

internal
bool isSceneUnloaded(const char *name)
{
	for (ListIterator itr = listGetIterator(&unloadedScenes);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		Scene *scene = *LIST_ITERATOR_GET_ELEMENT(Scene*, itr);
		if (!strcmp(name, scene->name))
		{
			return true;
		}
	}

	return false;
}

int32 unloadScene(const char *name)
{
	Scene *scene = getScene(name);

	if (scene && !isSceneUnloaded(name))
	{
		changeScene = true;
		listPushFront(&unloadedScenes, &scene);

		return 0;
	}

	return -1;
}

int32 shutdownScene(Scene **scene)
{
	sceneShutdownLua(&L, *scene);
	sceneShutdownSystems(*scene);

	return 0;
}

int32 deactivateScene(Scene *scene)
{
	for (ListIterator itr = listGetIterator(&activeScenes);
		!listIteratorAtEnd(itr);
		listMoveIterator(&itr))
	{
		Scene *activeScene = *LIST_ITERATOR_GET_ELEMENT(Scene*, itr);
		if (!strcmp(activeScene->name, scene->name))
		{
			listRemove(&activeScenes, &itr);
			return 0;
		}
	}

	return -1;
}

ComponentDefinition getComponentDefinition(
	Scene *scene,
	UUID name)
{
	ComponentDefinition *componentDefinition =
		(ComponentDefinition*)hashMapGetData(
			scene->componentDefinitions,
			&name);

	if (componentDefinition)
	{
		return *componentDefinition;
	}

	ComponentDefinition blankComponentDefinition;
	memset(&blankComponentDefinition, 0, sizeof(ComponentDefinition));
	return blankComponentDefinition;
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
	switch (type)
	{
		case DATA_TYPE_UINT8:
		case DATA_TYPE_INT8:
		case DATA_TYPE_CHAR:
		case DATA_TYPE_BOOL:
			return 1;
		case DATA_TYPE_UINT16:
		case DATA_TYPE_INT16:
			return 2;
		case DATA_TYPE_UINT32:
		case DATA_TYPE_INT32:
		case DATA_TYPE_FLOAT32:
			return 4;
		case DATA_TYPE_UINT64:
		case DATA_TYPE_INT64:
		case DATA_TYPE_FLOAT64:
			return 8;
		case DATA_TYPE_UUID:
			return UUID_LENGTH + 1;
		default:
			break;
	}

	return 0;
}

char* getDataTypeString(
	const ComponentValueDefinition *componentValueDefinition)
{
	char *dataTypeString = malloc(256);

	switch (componentValueDefinition->type)
	{
		case DATA_TYPE_UINT8:
			strcpy(dataTypeString, "uint8");
			break;
		case DATA_TYPE_UINT16:
			strcpy(dataTypeString, "uint16");
			break;
		case DATA_TYPE_UINT32:
			strcpy(dataTypeString, "uint32");
			break;
		case DATA_TYPE_UINT64:
			strcpy(dataTypeString, "uint64");
			break;
		case DATA_TYPE_INT8:
			strcpy(dataTypeString, "int8");
			break;
		case DATA_TYPE_INT16:
			strcpy(dataTypeString, "int16");
			break;
		case DATA_TYPE_INT32:
			strcpy(dataTypeString, "int32");
			break;
		case DATA_TYPE_INT64:
			strcpy(dataTypeString, "int64");
			break;
		case DATA_TYPE_FLOAT32:
			strcpy(dataTypeString, "float32");
			break;
		case DATA_TYPE_FLOAT64:
			strcpy(dataTypeString, "float64");
			break;
		case DATA_TYPE_BOOL:
			strcpy(dataTypeString, "bool");
			break;
		case DATA_TYPE_CHAR:
			strcpy(dataTypeString, "char");
			break;
		case DATA_TYPE_STRING:
			sprintf(
				dataTypeString,
				"char(%d)",
				componentValueDefinition->maxStringSize);
			break;
		case DATA_TYPE_UUID:
			strcpy(dataTypeString, "uuid");
			break;
		default:
			break;
	}

	return dataTypeString;
}

void exportEntitySnapshot(Scene *scene, UUID entity, const char *filename)
{
	cJSON *json = cJSON_CreateObject();

	cJSON_AddStringToObject(json, "uuid", entity.string);
	cJSON *jsonComponents = cJSON_AddObjectToObject(json, "components");

	List *components = (List*)hashMapGetData(scene->entities, entity.bytes);

	for (ListIterator itr = listGetIterator(components);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		UUID *componentUUID = LIST_ITERATOR_GET_ELEMENT(UUID, itr);
		cJSON *jsonComponent = cJSON_AddArrayToObject(
			jsonComponents,
			componentUUID->string);

		ComponentDataTable **componentDataTable =
			(ComponentDataTable**)hashMapGetData(
				scene->componentTypes,
				componentUUID->bytes);

		void *componentData = cdtGet((*componentDataTable), entity);

		ComponentDefinition componentDefinition = getComponentDefinition(
			scene,
			*componentUUID);

		uint32 bytesWritten = 0;
		for (uint32 i = 0; i < componentDefinition.numValues; i++)
		{
			ComponentValueDefinition *componentValueDefinition =
				&componentDefinition.values[i];

			uint32 size = 0;
			if (componentValueDefinition->type == DATA_TYPE_STRING)
			{
				size = componentValueDefinition->maxStringSize;
			}
			else
			{
				size = getDataTypeSize(componentValueDefinition->type);
			}

			uint32 paddingSize = size;
			if (componentValueDefinition->type == DATA_TYPE_STRING ||
				componentValueDefinition->type == DATA_TYPE_UUID)
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

			uint32 padding = bytesWritten % paddingSize;
			if (padding > 0)
			{
				bytesWritten += paddingSize - padding;
			}

			for (uint32 j = 0; j < componentValueDefinition->count; j++)
			{
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
					switch (componentValueDefinition->type)
					{
						case DATA_TYPE_UINT8:
							uint8Data = *(uint8*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)uint8Data);
							break;
						case DATA_TYPE_UINT16:
							uint16Data = *(uint16*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)uint16Data);
							break;
						case DATA_TYPE_UINT32:
							uint32Data = *(uint32*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)uint32Data);
							break;
						case DATA_TYPE_UINT64:
							uint64Data = *(uint64*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)uint64Data);
							break;
						case DATA_TYPE_INT8:
							int8Data = *(int8*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)int8Data);
							break;
						case DATA_TYPE_INT16:
							int16Data = *(int16*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)int16Data);
							break;
						case DATA_TYPE_INT32:
							int32Data = *(int32*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)int32Data);
							break;
						case DATA_TYPE_INT64:
							int64Data = *(int64*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)int64Data);
							break;
						case DATA_TYPE_FLOAT32:
							real32Data = *(real32*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)real32Data);
							break;
						case DATA_TYPE_FLOAT64:
							real64Data = *(real64*)valueData;
							cJSON_AddNumberToObject(
								jsonComponentValue,
								dataTypeString,
								(double)real64Data);
							break;
						case DATA_TYPE_BOOL:
							boolData = *(bool*)valueData;
							cJSON_AddBoolToObject(
								jsonComponentValue,
								dataTypeString,
								(cJSON_bool)boolData);
							break;
						case DATA_TYPE_CHAR:
							charData[0] = *(char*)valueData;
							charData[1] = '\0';
							cJSON_AddStringToObject(
								jsonComponentValue,
								dataTypeString,
								charData);
							break;
						case DATA_TYPE_STRING:
						case DATA_TYPE_UUID:
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

					switch (componentValueDefinition->type)
					{
						case DATA_TYPE_UINT8:
							uint8Data = *(uint8*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)uint8Data);
							break;
						case DATA_TYPE_UINT16:
							uint16Data = *(uint16*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)uint16Data);
							break;
						case DATA_TYPE_UINT32:
							uint32Data = *(uint32*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)uint32Data);
							break;
						case DATA_TYPE_UINT64:
							uint64Data = *(uint64*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)uint64Data);
							break;
						case DATA_TYPE_INT8:
							int8Data = *(int8*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)int8Data);
							break;
						case DATA_TYPE_INT16:
							int16Data = *(int16*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)int16Data);
							break;
						case DATA_TYPE_INT32:
							int32Data = *(int32*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)int32Data);
							break;
						case DATA_TYPE_INT64:
							int64Data = *(int64*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)int64Data);
							break;
						case DATA_TYPE_FLOAT32:
							real32Data = *(real32*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)real32Data);
							break;
						case DATA_TYPE_FLOAT64:
							real64Data = *(real64*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateNumber((double)real64Data);
							break;
						case DATA_TYPE_BOOL:
							boolData = *(bool*)valueData;
							jsonComponentValueDataArrayItem =
								cJSON_CreateBool((cJSON_bool)boolData);
							break;
						case DATA_TYPE_CHAR:
							charData[0] = *(char*)valueData;
							charData[1] = '\0';
							jsonComponentValueDataArrayItem =
								cJSON_CreateString(charData);
							break;
						case DATA_TYPE_STRING:
						case DATA_TYPE_UUID:
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

				bytesWritten += size;
			}

			free(dataTypeString);
			cJSON_AddItemToArray(jsonComponent, jsonComponentValue);
		}
	}

	writeJSON(json, filename);
	cJSON_Delete(json);
}

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
		char *systemName = LIST_ITERATOR_GET_ELEMENT(UUID, itr)->string;
		systemNames[i] = malloc(strlen(systemName) + 1);
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

void exportSceneSnapshot(Scene *scene, const char *filename)
{
	LOG("Exporting scene (%s)...\n", scene->name);

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

	systemNames = getSystemNames(
		(List*)&scene->physicsFrameSystems,
		&numSystemNames);

	cJSON *internalUpdateSystems = cJSON_CreateStringArray(
		(const char**)systemNames,
		numSystemNames);
	freeSystemNames(systemNames, numSystemNames);

	cJSON_AddItemToObject(updateSystems, "internal", internalUpdateSystems);

	cJSON *drawSystems = cJSON_AddObjectToObject(systems, "draw");

	systemNames = getSystemNames(
		(List*)&scene->luaRenderFrameSystemNames,
		&numSystemNames);

	cJSON *externalDrawSystems = cJSON_CreateStringArray(
		(const char**)systemNames,
		numSystemNames);
	freeSystemNames(systemNames, numSystemNames);

	cJSON_AddItemToObject(drawSystems, "external", externalDrawSystems);

	systemNames = getSystemNames(
		(List*)&scene->renderFrameSystems,
		&numSystemNames);

	cJSON *internalDrawSystems = cJSON_CreateStringArray(
		(const char**)systemNames,
		numSystemNames);
	freeSystemNames(systemNames, numSystemNames);

	cJSON_AddItemToObject(drawSystems, "internal", internalDrawSystems);

	cJSON *componentLimits = cJSON_AddObjectToObject(json, "component_limits");

	for (uint32 i = 0; i < scene->numComponentLimitNames; i++)
	{
		for (HashMapIterator itr = hashMapGetIterator(scene->componentTypes);
			!hashMapIteratorAtEnd(itr);
			hashMapMoveIterator(&itr))
		{
			UUID *componentUUID = (UUID*)hashMapIteratorGetKey(itr);
			if (!strcmp(scene->componentLimitNames[i], componentUUID->string))
			{
				uint32 componentLimit =
					(*(ComponentDataTable**)hashMapIteratorGetValue(itr))
						->numEntries;
				cJSON_AddNumberToObject(
					componentLimits,
					componentUUID->string,
					componentLimit);
			}
		}
	}

	cJSON_AddStringToObject(json, "active_camera", scene->mainCamera.string);
	cJSON_AddNumberToObject(json, "gravity", scene->gravity);

	writeJSON(json, filename);
	cJSON_Delete(json);

	LOG("Successfully exported scene (%s)\n", scene->name);
}

inline
void sceneAddRenderFrameSystem(
	Scene *scene,
	UUID system)
{
	listPushBack(&scene->renderFrameSystems, &system);
}

inline
void sceneAddPhysicsFrameSystem(
	Scene *scene,
	UUID system)
{
	listPushBack(&scene->physicsFrameSystems, &system);
}

void sceneInitRenderFrameSystems(Scene *scene)
{
	for (ListIterator itr = listGetIterator(&scene->renderFrameSystems);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		UUID *systemName = LIST_ITERATOR_GET_ELEMENT(UUID, itr);
		System *system = hashMapGetData(systemRegistry, systemName);

		if (!system)
		{
			LOG("System %s doesn't exist in system registry\n",
				systemName->string);
			continue;
		}

		if (system->init != 0)
		{
			system->init(scene);
		}
	}
}

void sceneInitPhysicsFrameSystems(Scene *scene)
{
	for (ListIterator itr = listGetIterator(&scene->physicsFrameSystems);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		UUID *systemName = LIST_ITERATOR_GET_ELEMENT(UUID, itr);
		System *system = hashMapGetData(systemRegistry, systemName);

		if (!system)
		{
			LOG("System %s doesn't exist in system registry\n",
				systemName->string);
			continue;
		}

		if (system->init != 0)
		{
			system->init(scene);
		}
	}
}

void sceneInitSystems(Scene *scene)
{
	sceneInitRenderFrameSystems(scene);
	sceneInitPhysicsFrameSystems(scene);
}

void sceneRunRenderFrameSystems(Scene *scene, real64 dt)
{
	for (ListIterator itr = listGetIterator(&scene->renderFrameSystems);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		UUID *systemName = LIST_ITERATOR_GET_ELEMENT(UUID, itr);
		System *system = hashMapGetData(systemRegistry, systemName);

		if (!system)
		{
			LOG("System %s doesn't exist in system registry\n",
				systemName->string);
			continue;
		}

		systemRun(scene, system, dt);
	}
}

void sceneRunPhysicsFrameSystems(Scene *scene, real64 dt)
{
	for (ListIterator itr = listGetIterator(&scene->physicsFrameSystems);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		UUID *systemName = LIST_ITERATOR_GET_ELEMENT(UUID, itr);
		System *system = hashMapGetData(systemRegistry, systemName);

		if (!system)
		{
			LOG("System %s doesn't exist in system registry\n",
				systemName->string);
			continue;
		}

		systemRun(scene, system, dt);
	}
}

void sceneShutdownRenderFrameSystems(Scene *scene)
{
	for (ListIterator itr = listGetIterator(&scene->renderFrameSystems);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		UUID *systemName = LIST_ITERATOR_GET_ELEMENT(UUID, itr);
		System *system = hashMapGetData(systemRegistry, systemName);

		if (!system)
		{
			LOG("System %s doesn't exist in system registry\n",
				systemName->string);
			continue;
		}

		if (system->shutdown != 0)
		{
			system->shutdown(scene);
		}
	}
}

void sceneShutdownPhysicsFrameSystems(Scene *scene)
{
	for (ListIterator itr = listGetIterator(&scene->physicsFrameSystems);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		UUID *systemName = LIST_ITERATOR_GET_ELEMENT(UUID, itr);
		System *system = hashMapGetData(systemRegistry, systemName);

		if (!system)
		{
			LOG("System %s doesn't exist in system registry\n",
				systemName->string);
			continue;
		}

		if (system->shutdown != 0)
		{
			system->shutdown(scene);
		}
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
		LOG("Lua error: %s\n", lua_tostring(*L, -1));
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
		LOG("Lua error: %s\n", lua_tostring(*L, -1));
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
		componentID,
		maxComponents,
		componentSize);

	ASSERT(table);

	hashMapInsert(scene->componentTypes, &componentID, &table);

	ASSERT(hashMapGetData(scene->componentTypes, &componentID));
	ASSERT(table == *(ComponentDataTable **)
		hashMapGetData(
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
		ListIterator litr = listGetIterator(
			(List *)hashMapIteratorGetValue(itr));
		while (!listIteratorAtEnd(litr))
		{
			// Remove this component from that entity
			if (!strcmp(
				LIST_ITERATOR_GET_ELEMENT(UUID, litr)->string,
				componentID.string))
			{
				listRemove((List *)hashMapIteratorGetValue(itr), &litr);
			}
			listMoveIterator(&litr);
		}

		hashMapMoveIterator(&itr);
	}

	// Delete the component data table
	ComponentDataTable **temp = (ComponentDataTable **)hashMapGetData(
		scene->componentTypes,
		&componentID);

	if (temp)
	{
		freeComponentDataTable(temp);
	}
}

void sceneRegisterEntity(Scene *s, UUID newEntity)
{
#ifdef _DEBUG
	List *entityList;
	if ((entityList = hashMapGetData(s->entities, &newEntity)))
	{
		LOG(
			"Entity %s already exists in scene %s\n",
			newEntity.string,
			s->name);

		listClear(entityList);
		hashMapDelete(s->entities, &newEntity);

		//ASSERT(false && "Entity already exists in scene");
	}
#endif

	List emptyList = createList(sizeof(UUID));
	hashMapInsert(s->entities, &newEntity, &emptyList);
}

UUID sceneCreateEntity(Scene *s)
{
	UUID newEntity = generateUUID();
	sceneRegisterEntity(s, newEntity);
	return newEntity;
}

void sceneRemoveEntityComponents(Scene *s, UUID entity)
{
	List *entityComponentList = hashMapGetData(s->entities, &entity);

	if (!entityComponentList)
	{
		return;
	}

	// For each component type
	while (entityComponentList->front)
	{
		sceneRemoveComponentFromEntity(
			s,
			entity,
			*(UUID *)entityComponentList->front->data);
	}

	// Clear component list
	listClear(entityComponentList);
}

void sceneRemoveEntity(Scene *s, UUID entity)
{
	UUID transformComponentID = idFromName("transform");
	TransformComponent *transform =
		(TransformComponent*)sceneGetComponentFromEntity(
			s,
			entity,
			transformComponentID);

	// Loop through all the children and delete them
	if (transform && transform->firstChild.string[0] != 0)
	{
		UUID child = transform->firstChild;
		UUID sibling = {};
		while (child.string[0] != 0)
		{
			transform = (TransformComponent *)sceneGetComponentFromEntity(
				s,
				child,
				transformComponentID);

			sibling = transform->nextSibling;

			sceneRemoveEntity(s, child);
			child = sibling;
		}
	}

	sceneRemoveEntityComponents(s, entity);
	hashMapDelete(s->entities, &entity);
}

int32 sceneAddComponentToEntity(
	Scene *s,
	UUID entity,
	UUID componentType,
	void *componentData)
{
	// LOG("Adding %s component to entity with id %s\n",
	// 	componentType.string,
	// 	entity.string);

	// Get the data table
	ComponentDataTable **dataTable = hashMapGetData(
		s->componentTypes,
		&componentType);

	if (!dataTable || !*dataTable)
	{
		return -1;
	}

	List *l = hashMapGetData(s->entities, &entity);

	// Add the component to the data table
	if(cdtInsert(
		   *dataTable,
		   entity,
		   componentData) == -1)
	{
		return -1;
	}

	if (listContains(l, &componentType))
	{
		LOG("\n\nOverwriting component data of %s on entity %s\n\n\n",
			componentType.string,
			entity.string);
	}
	else
	{
		// Add the component type to the list
		listPushBack(l, &componentType);
	}

	return 0;
}

void sceneRemoveComponentFromEntity(
	Scene *s,
	UUID entity,
	UUID componentType)
{
	ComponentDataTable **table = hashMapGetData(
		s->componentTypes,
		&componentType);

	if (!table || !*table)
	{
		return;
	}

	// NOTE(Joshua): So this is where I'd want a generic component
	//               cleanup function, but we can't have that with how
	//               we load components.

	// Check to ensure that the component has been properly freed
	if (!strcmp(componentType.string, "model"))
	{
		freeModel(((ModelComponent *)cdtGet(*table, entity))->name);
	}
	if (!strcmp(componentType.string, "rigid_body"))
	{
		destroyRigidBody((RigidBodyComponent *)cdtGet(*table, entity));
	}

	cdtRemove(*table, entity);

	List *componentTypeList = hashMapGetData(s->entities, &entity);

	if (!componentTypeList)
	{
		return;
	}

	listRemoveData(componentTypeList, &componentType);
}

void *sceneGetComponentFromEntity(
	Scene *s,
	UUID entity,
	UUID componentType)
{
	ComponentDataTable **table = hashMapGetData(
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
	UUID uuid = {};
	if (name)
	{
		strcpy(uuid.string, name);
	}

	return uuid;
}
