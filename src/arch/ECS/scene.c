#include "ECS/scene.h"
#include "ECS/component.h"
#include "ECS/system.h"

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

	ret->physicsFrameSystems = createList(sizeof(System));
	ret->renderFrameSystems = createList(sizeof(System));

	return ret;
}

void freeScene(Scene **scene)
{
	sceneShutdownSystems(*scene);
	listClear(&(*scene)->physicsFrameSystems);
	listClear(&(*scene)->renderFrameSystems);

	HashMapIterator itr = hashMapGetIterator((*scene)->entities);
	while (!hashMapIteratorAtEnd(itr))
	{
		sceneRemoveEntity(*scene, *(UUID *)hashMapIteratorGetKey(itr));
		hashMapMoveIterator(&itr);
	}

	itr = hashMapGetIterator((*scene)->componentTypes);
	while (!hashMapIteratorAtEnd(itr))
	{
		sceneRemoveComponentType(*scene, *(UUID *)hashMapIteratorGetKey(itr));
		hashMapMoveIterator(&itr);
	}

	freeHashMap(&(*scene)->entities);
	freeHashMap(&(*scene)->componentTypes);

	free(*scene);
	*scene = 0;
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

void sceneRunRenderFrameSystems(Scene *scene)
{
	ListIterator itr = listGetIterator(&scene->renderFrameSystems);
	while (!listIteratorAtEnd(itr))
	{
		systemRun(scene, LIST_ITERATOR_GET_ELEMENT(System, itr));

		listMoveIterator(&itr);
	}
}

void sceneRunPhysicsFrameSystems(Scene *scene)
{
	ListIterator itr = listGetIterator(&scene->physicsFrameSystems);
	while (!listIteratorAtEnd(itr))
	{
		systemRun(scene, LIST_ITERATOR_GET_ELEMENT(System, itr));

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

inline
UUID idFromName(const char *name)
{
	UUID ret = {};
	strcpy(ret.string, name);
	return ret;
}
