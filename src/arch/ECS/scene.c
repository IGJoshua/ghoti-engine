#include "ECS/scene.h"
#include "ECS/component.h"
#include "ECS/system.h"

#include "data/hash_map.h"
#include "data/list.h"

#include "file/utilities.h"

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

	FILE *file = fopen(sceneFilename, "rb");

	if (file)
	{
		*scene = createScene();

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
	DIR *dir = opendir(sceneFolder);

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
				sceneFolder);

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
		printf("Failed to open %s\n", sceneFolder);
		error = -1;
	}

	free(sceneFolder);

	return error;
}

void freeScene(Scene **scene)
{
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
	listPushFront(hashMapGetKey(s->entities, &entity), &componentType);
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
