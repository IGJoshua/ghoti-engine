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
	// Get the first component type
	ComponentDataTable *firstComp = (ComponentDataTable *)system->componentTypes.front->data;

	// For each entity in the component table
	for (uint32 i = 0; i < firstComp->numEntries; ++i)
	{
		// Check to see if it has the other component types
		int32 entityValid = 1;

		ListNode **itr = listGetIterator(&system->componentTypes);
		for (listMoveIterator(&itr);
			 !listIteratorAtEnd(itr);
			 listMoveIterator(&itr))
		{
			// Get the component to check
			UUID *componentID = (UUID *)(*itr)->data;
			ComponentDataTable *table = hashMapGetKey(scene->componentTypes, componentID);

			// Check if the entity exists in the table
			uint32 *entityIndex =
				hashMapGetKey(
					table->idToIndex,
					firstComp->data + i * (firstComp->componentSize * sizeof(UUID))
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
			system->fn(scene, *(UUID *)(firstComp->data + i * (firstComp->componentSize * sizeof(UUID))));
		}
	}
}
