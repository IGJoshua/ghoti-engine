#include "ECS/scene.h"
#include "ECS/component.h"

#include "data/hash_map.h"
#include "data/list.h"

#include <malloc.h>
#include <string.h>
#include <stdlib.h>

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

	return ret;
}

void freeScene(Scene **scene)
{
	freeHashMap(&(*scene)->componentTypes);
	freeHashMap(&(*scene)->entities);
	free(*scene);
	*scene = 0;
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
		ret.bytes[i] = (rand() % 255) + 1;
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
