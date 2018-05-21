#include "ECS/scene.h"

#include "ECS/component.h"

#include "data/hash_map.h"
#include "data/list.h"

#include <malloc.h>
#include <string.h>
#include <stdlib.h>

Scene *createScene()
{
	Scene *ret = malloc(sizeof(Scene));

	ret->componentTypes = createHashMap(
		sizeof(UUID),
		sizeof(ComponentDataTable *),
		COMPONENT_TYPE_BUCKETS,
		(ComparisonOp)&strcmp
	);
	ret->entities = createHashMap(sizeof(UUID), sizeof(List), ENTITY_BUCKETS, (ComparisonOp)&strcmp);

	return ret;
}

void freeScene(Scene **scene)
{
	freeHashMap(&(*scene)->componentTypes);
	freeHashMap(&(*scene)->entities);
	free(*scene);
	*scene = 0;
}

void sceneAddComponentType(Scene *scene, UUID componentID, uint32 componentSize, uint32 maxComponents)
{
	ComponentDataTable *table = createComponentDataTable(maxComponents, componentSize);
	ASSERT(table);
	hashMapInsert(scene->componentTypes, &componentID, &table);
	ASSERT(hashMapGetKey(scene->componentTypes, &componentID));
	ASSERT(table == *(ComponentDataTable **)hashMapGetKey(scene->componentTypes, &componentID));
}

void sceneRemoveComponentType(Scene *scene, UUID componentID)
{
	ComponentDataTable *temp = hashMapGetKey(scene->componentTypes, &componentID);
	freeComponentDataTable(&temp);
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

UUID sceneCreateEntity(Scene *s)
{
	UUID newEntity = generateUUID();

	List emptyList = createList(sizeof(UUID));
	hashMapInsert(s->entities, &newEntity, &emptyList);

	return newEntity;
}

void sceneRemoveEntity(Scene *s, UUID entity)
{
	List *entityComponentList = hashMapGetKey(s->entities, &entity);

	// For each component type
	for (ListNode **listIterator = listGetIterator(entityComponentList);
		 !listIteratorAtEnd(listIterator);
		 listMoveIterator(&listIterator))
	{
		// Remove this entity from the components
		cdtRemove(
			hashMapGetKey(s->componentTypes, (*listIterator)->data),
			entity
		);
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
	ComponentDataTable *dataTable = hashMapGetKey(s->componentTypes, &componentType);
	ASSERT(dataTable);

	// Add the component to the data table
	cdtInsert(
		dataTable,
		entity,
		componentData
	);
	// Add the component type to the list
	listPushFront(hashMapGetKey(s->entities, &entity), &componentType);
}

void sceneRemoveComponentFromEntity(
	Scene *s,
	UUID entity,
	UUID componentType)
{
	cdtRemove(hashMapGetKey(s->componentTypes, &componentType), entity);

	List *componentTypeList = hashMapGetKey(s->entities, &entity);
	for (ListNode **itr = listGetIterator(componentTypeList);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		if (strcmp(((UUID *)(*itr)->data)->string, componentType.string))
		{
			listRemove(componentTypeList, itr);
		}
	}
}

inline
void *sceneGetComponentFromEntity(
	Scene *s,
	UUID entity,
	UUID componentType)
{
	return cdtGet(hashMapGetKey(s->componentTypes, &componentType), entity);
}
