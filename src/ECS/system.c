#include "ECS/system.h"

#include "data/list.h"
#include "data/hash_map.h"

#include <string.h>

inline
System createSystem(
	List components,
	InitSystem init,
	SystemFn fn,
	ShutdownSystem shutdown)
{
	System sys;
	sys.componentTypes = components;
	sys.init = init;
	sys.fn = fn;
	sys.shutdown = shutdown;
	return sys;
}

void systemRun(
	Scene *scene,
	System *system)
{
	// Return if it's not a system which runs each frame
	if (!system->fn)
	{
		return;
	}

	// Get the first component type
	ComponentDataTable *firstComp = *(ComponentDataTable **)
		hashMapGetKey(
			scene->componentTypes,
			(UUID*)system->componentTypes.front->data
		);

	UUID emptyID = {};

	// For each entity in the component table
	for (uint32 i = 0; i < firstComp->numEntries; ++i)
	{
		if (!strcmp(
				emptyID.string,
				// NOTE(Joshua): "Feels like pretty standard C to me"
				((UUID *)(firstComp->data + i * (firstComp->componentSize + sizeof(UUID))))->string
			))
		{
			continue;
		}

		// Check to see if it has the other component types
		int32 entityValid = 1;

		ListNode **itr = listGetIterator(&system->componentTypes);
		for (listMoveIterator(&itr);
			 !listIteratorAtEnd(itr);
			 listMoveIterator(&itr))
		{
			// Get the component to check
			UUID *componentID = (UUID *)(*itr)->data;
			ComponentDataTable *table = *(ComponentDataTable **)hashMapGetKey(scene->componentTypes, componentID);

			// Check if the entity exists in the table
			uint32 *entityIndex =
				hashMapGetKey(
					table->idToIndex,
					firstComp->data + i * (firstComp->componentSize + sizeof(UUID))
				);

			if (!entityIndex)
			{
				entityValid = 0;
				break;
			}
		}

		// Call the function
		if (entityValid)
		{
			system->fn(scene, *(UUID *)(firstComp->data + i * (firstComp->componentSize + sizeof(UUID))));
		}
	}
}

void freeSystem(System *system)
{
	listClear(&system->componentTypes);
	system->init = 0;
	system->fn = 0;
	system->shutdown = 0;
}